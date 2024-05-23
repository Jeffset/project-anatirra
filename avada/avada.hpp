/* Copyright 2020-2024 Fedor Ihnatkevich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "avada/buffer.hpp"
#include "avada/input.hpp"
#include "base/macro.hpp"

#include "avada/config.hpp"

#include <signal.h>
#include <chrono>
#include <memory>

struct termios;

namespace avada {

class AVADA_PUBLIC Context {
 public:
  Context() /* may throw */;
  ~Context() noexcept;

  DISABLE_COPY_MOVE(Context);

  input::Event poll_event(std::chrono::milliseconds timeout) /* may throw */;

  void render() /* may throw */;

  GETTER int get_rows() const noexcept { return rows_; }
  GETTER int get_columns() const noexcept { return columns_; }

  GETTER render::Buffer& render_buffer() noexcept { return back_buffer_; }
  GETTER const render::Buffer& render_buffer() const noexcept { return back_buffer_; }

 private:
  AVADA_PRIVATE void update_size() /* may throw */;

  class AVADA_PRIVATE ScopedPrivateModeChange {
   public:
    ScopedPrivateModeChange(std::initializer_list<int> to_enable,
                            std::initializer_list<int> to_disable) /* may throw */;

    ~ScopedPrivateModeChange() noexcept;

    DISABLE_COPY_MOVE(ScopedPrivateModeChange);

   private:
    static void format_control_sequence(std::ostream& os,
                                        const std::vector<int>& modes,
                                        char action) noexcept;

   private:
    std::vector<int> to_enable_;
    std::vector<int> to_disable_;
  };

 private:
  sighandler_t saved_sigwinch_;
  std::unique_ptr<termios> saved_context_;
  ScopedPrivateModeChange private_mode_changer_;

  input::InputParser input_parser_;

  render::Buffer front_buffer_;
  render::Buffer back_buffer_;

  int rows_;
  int columns_;
};

}  // namespace avada
