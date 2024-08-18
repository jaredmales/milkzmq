/*
 * Copyright (c) 2024 The Arizona Board of Regents on behalf of the
 * University of Arizona. All rights reserved.
 *
 * This file is part of CHAI.
 *
 * CHAI is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * CHAI is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with CHAI. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <ImageStreamIO/ImageStreamIO.h>

#include <cstring>
#include <filesystem>
#include <unordered_map>

namespace chai {
namespace masala {

class ImStream3 {
 private:
  int semNum;
  bool init;
  std::string name;
  IMAGE *im;
  std::size_t width, height, datatype, sizeBytes;

  static std::unordered_map<std::size_t, std::size_t> typeToSizeMap;

  //! Open an existing image stream.
  void open();

  //! Create an image stream with the parameters we have.
  void create();

 public:
  //! Constructor.
  ImStream3(std::string name, std::size_t width = 0, std::size_t height = 0, std::size_t datatype = _DATATYPE_UNINITIALIZED);

  //! Destructor.
  ~ImStream3();

  //! Simple blocking read on an image stream.
  int read(void *p, struct timespec *atime);

  //! A spinning read on an image stream.
  int read_spin(void *p, struct timespec *atime);

  //! Non-blocking read, i.e. read whatever is in the buffer.
  int read_non_blocking(void *p, struct timespec *atime);

  //! Cancel a blocking read. Be careful because you might wake up others.
  void cancel_blocking_read();

  //! Write into an image stream.
  int send(void *p, struct timespec atime = {0, 0});
};

}  // namespace masala
}  // namespace chai

#ifndef ALL_SEMAPHORES
#define ALL_SEMAPHORES (-1)
#endif

#ifndef MD_WRITE_START
#define MD_WRITE_START (1)
#endif

#ifndef MD_WRITE_DONE
#define MD_WRITE_DONE (0)
#endif
