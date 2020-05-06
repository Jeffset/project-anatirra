// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_AVADA_AVADA
#define ANATIRRA_AVADA_AVADA

#include "avada/buffer.hpp"
#include "avada/input.hpp"

#include <codecvt>
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <string>

struct termios;

namespace avada {

class Context {
 public:
  Context() /* may throw */;
  ~Context() noexcept;

  input::Event poll_event() /* may throw */;

  void render() /* may throw */;

  int get_rows() const noexcept { return rows_; }
  int get_columns() const noexcept { return columns_; }

  render::Buffer& render_buffer() noexcept { return back_buffer_; }
  const render::Buffer& render_buffer() const noexcept { return back_buffer_; }

 private:
  void update_size() /* may throw */;

  class PrivateModeChanger {
   public:
    void apply(std::initializer_list<int> to_enable,
               std::initializer_list<int> to_disable) /* may throw */;
    ~PrivateModeChanger() noexcept;

   private:
    static void format_control_sequence(std::ostream& os,
                                        const std::vector<int>& modes,
                                        char action) noexcept;

   private:
    std::vector<int> to_enable_;
    std::vector<int> to_disable_;
  };

 private:
  std::unique_ptr<termios> saved_context_;
  PrivateModeChanger private_mode_changer_;

  input::InputParser input_parser_;

  render::Buffer front_buffer_;
  render::Buffer back_buffer_;

  int rows_;
  int columns_;
};

}  // namespace avada

#endif  // ANATIRRA_AVADA_AVADA