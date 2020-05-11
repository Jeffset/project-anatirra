// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/avada.hpp"

#include "base/debug/debug.hpp"
#include "base/exception.hpp"
#include "base/macro.hpp"

#include <csignal>
#include <regex>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <termios.h>
#include <unistd.h>

#define ESC_CH "\x1B"

namespace avada {

namespace {

#define SYSTEM_CALL_NON_ZERO(call) \
  if ((call) != 0)                 \
  throw ::base::system_exception(#call)

template <int N>
inline void write_stdout(const char (&data)[N]) {
  ::write(STDOUT_FILENO, data, N);
}

volatile std::sig_atomic_t g_pending_resize = 0;
Context* g_avada_context;

extern "C" void handle_resize(int) {
  g_pending_resize = 1;
}

}  // namespace

Context::Context() {
  ASSERT(g_avada_context == nullptr) << "Only one AvadaContext is permitted to exist";
  std::signal(SIGWINCH, handle_resize);

  std::setlocale(LC_ALL, "");

  saved_context_ = std::make_unique<termios>();
  SYSTEM_CALL_NON_ZERO(::tcgetattr(STDIN_FILENO, saved_context_.get()));

  auto raw = *saved_context_;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  // block to have at least a single byte (actually, we use poll to wait)
  raw.c_cc[VMIN] = 1;
  // do not block in terms of time.
  raw.c_cc[VTIME] = 0;

  SYSTEM_CALL_NON_ZERO(::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw));

  private_mode_changer_.apply(
      {
          // Enable:
          1000,  // Send Mouse X & Y on button press and release.
          1006,  // Report Mouse Move.
          1003,  // Use All Motion Mouse Tracking.
          1049,  // Save cursor and use Alternate Screen Buffer, clearing it first.
          2004,  // Set bracketed paste mode. TODO: support bracketed paste mode.
      },
      {
          // Disable:
          7,     // No Wraparound Mode.
          45,    // No Reverse-wraparound Mode.
          25,    // Hide cursor.
          30,    // Don't show scrollbar.
          1010,  // Don’t scroll to bottom on tty output (rxvt).
          1011,  // Don’t scroll to bottom on key press (rxvt).
      });

  write_stdout(
      "\x1b[H"  // Position at (0,0)
      "\x1b%G"  // UTF-8
      "\x1b=");

  update_size();

  // Must be the last line.
  g_avada_context = this;
}

Context::~Context() noexcept {
  std::signal(SIGWINCH, SIG_DFL);
  ::tcsetattr(STDIN_FILENO, TCSAFLUSH, saved_context_.get());
  ASSERT(g_avada_context == this);
  g_avada_context = nullptr;
}

input::Event Context::poll_event() {
  pollfd pfd{STDIN_FILENO, POLLIN, 0};

  int poll_result = 0;
  while (poll_result == 0) {
    poll_result = ::poll(&pfd, 1, 1 /*ms*/);
  }
  if (poll_result == -1) {
    if (errno == EINTR && g_pending_resize) {
      // We've caught SIGWINCH, it's ok.
      g_pending_resize = 0;
      update_size();
      return input::ResizeEvent{columns_, rows_};
    }
    // Otherwise it's not ok.
    throw base::system_exception("'poll' operation failed");
  }

  // Now data is ready, read it.
  char raw_data[128] = {0};
  ssize_t n_read = 0;
  while (n_read <= 0) {
    n_read = ::read(STDIN_FILENO, &raw_data, sizeof(raw_data));
    if (n_read == -1)
      throw base::system_exception("'read' operation failed");
  }

  std::string_view data{raw_data, static_cast<size_t>(n_read)};

  return input_parser_.parse_event(data);
}

void Context::render() {
  back_buffer_.render(front_buffer_);
}

void Context::update_size() {
  struct winsize ws;
  SYSTEM_CALL_NON_ZERO(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws));
  rows_ = ws.ws_row;
  columns_ = ws.ws_col;
  back_buffer_.resize(rows_, columns_);
}

void Context::ScopedPrivateModeChange::apply(std::initializer_list<int> to_enable,
                                             std::initializer_list<int> to_disable) {
  to_enable_ = to_enable;
  to_disable_ = to_disable;

  std::ostringstream oss;
  // Change modes.
  format_control_sequence(oss, to_enable_, 'h');
  format_control_sequence(oss, to_disable_, 'l');
  auto sequence = oss.str();
  LOG() << "Change mode sequence: " << internal::escape_for_log(sequence);
  auto result = ::write(STDOUT_FILENO, sequence.data(), sequence.size());
  if (result < 0)
    throw base::system_exception("'write' operation failed");
  if (static_cast<size_t>(result) != sequence.size())
    throw base::exception("'write' wrote less than expected");
}

Context::ScopedPrivateModeChange::~ScopedPrivateModeChange() noexcept {
  std::ostringstream oss;
  // Restore saved modes.
  format_control_sequence(oss, to_enable_, 'l');
  format_control_sequence(oss, to_disable_, 'h');
  auto sequence = oss.str();
  LOG() << "Restore mode sequence: " << internal::escape_for_log(sequence);
  ::write(STDOUT_FILENO, sequence.data(), sequence.size());
}

// static
void Context::ScopedPrivateModeChange::format_control_sequence(
    std::ostream& os,
    const std::vector<int>& modes,
    char action) noexcept {
  if (modes.empty())
    return;

  os << ESC_CH "[?";
  bool first = true;
  for (int mode : modes) {
    if (!first) {
      os << ";";
    } else {
      first = false;
    }
    os << mode;
  }
  os << action;
}

}  // namespace avada
