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

#include "ims3.h"

std::unordered_map<std::size_t, std::size_t> chai::masala::ImStream3::typeToSizeMap = {
    {_DATATYPE_UINT8, SIZEOF_DATATYPE_UINT8},   {_DATATYPE_UINT16, SIZEOF_DATATYPE_UINT16}, {_DATATYPE_UINT32, SIZEOF_DATATYPE_UINT32},
    {_DATATYPE_UINT64, SIZEOF_DATATYPE_UINT64}, {_DATATYPE_INT8, SIZEOF_DATATYPE_INT8},     {_DATATYPE_INT16, SIZEOF_DATATYPE_INT16},
    {_DATATYPE_INT32, SIZEOF_DATATYPE_INT32},   {_DATATYPE_INT64, SIZEOF_DATATYPE_INT64},   {_DATATYPE_FLOAT, SIZEOF_DATATYPE_FLOAT},
    {_DATATYPE_DOUBLE, SIZEOF_DATATYPE_DOUBLE}};

chai::masala::ImStream3::ImStream3(std::string name, std::size_t width, std::size_t height, std::size_t datatype)
    : init(false), name(name), im(nullptr), width(width), height(height), datatype(datatype) {
  std::string path = "/milk/shm/" + name + ".im.shm";
  if (std::filesystem::exists(path)) {
    open();  // If the image stream already exists, open it.
  } else {
    if (0 == width || height == 0 || datatype == _DATATYPE_UNINITIALIZED) {
      throw new std::runtime_error("ims3::ctor insufficient parameters");
    } else {
      create();  // If it doesn't and we have the parameters, make it.
    }
  }
  semNum = ImageStreamIO_getsemwaitindex(im, semNum);  // Get a semaphore number.
  init = true;                                         // If we make it here, we're initialized.
}

chai::masala::ImStream3::~ImStream3() {
  if (im != nullptr) {
    ImageStreamIO_closeIm(im);
    delete im;
    im = nullptr;
  }
}

void chai::masala::ImStream3::open() {
  im = new IMAGE();  // Open the image stream.
  if (im == nullptr) throw new std::runtime_error("ims3::open new");
  if (0 != ImageStreamIO_openIm(im, name.c_str())) throw new std::runtime_error("ims3::open openIm");

  datatype = im->md->datatype;  // Get metadata.
  width = im->md->size[0];
  height = im->md->naxis == 1 ? 1 : im->md->size[1];
  sizeBytes = width * height * typeToSizeMap[datatype];
}

void chai::masala::ImStream3::create() {
  im = new IMAGE();                                                     // Make room for an image stream object.
  if (im == nullptr) throw new std::runtime_error("ims3::create new");  // Check that everything's okay so far.
  std::size_t naxis = 2;                                                // We're going to assume two dimensions.
  uint32_t imsize[naxis] = {(uint32_t)width, (uint32_t)height};         // Still assuming two dims.
  sizeBytes = width * height * typeToSizeMap[datatype];                 // Record the size in bytes.
  if (0 != ImageStreamIO_createIm_gpu(im, name.c_str(), naxis, imsize,  // Create the stream.
                                      datatype, -1, 1, 10, 0,           //
                                      IMG_SENT | ZAXIS_UNDEF, 1)) {     //
    throw new std::runtime_error("ims3::create createIm");              // Tell everyone if something goes wrong.
  }
}

int chai::masala::ImStream3::read(void *__restrict__ p, struct timespec *__restrict__ atime) {
  if (!init) return ENOENT;                      // If we aren't initialzied, don't even try.
  if (p == nullptr) return EINVAL;               // Try not to segfault.
  uint64_t count = im->md->cnt0;                 // Get the count before we wait.
  ImageStreamIO_semwait(im, semNum);             // Wait until someone writes to the stream.
  memcpy(p, im->array.raw, sizeBytes);           // Copy to the buffer.
  if (atime != nullptr) *atime = im->md->atime;  // If requested, copy the time.
  return im->md->cnt0 - count;                   // Return the difference to see if we missed.
}

int chai::masala::ImStream3::read_spin(void *__restrict__ p, struct timespec *__restrict__ atime) {
  if (!init) return ENOENT;                      // If we aren't initialzied, don't even try.
  if (p == nullptr) return EINVAL;               // Try not to segfault.
  uint64_t count = im->md->cnt0;                 // Get the current count.
  while (count == im->md->cnt0) sched_yield();   // Wait until it changes.
  memcpy(p, im->array.raw, sizeBytes);           // Copy to the buffer.
  if (atime != nullptr) *atime = im->md->atime;  // If requested, copy the time.
  return 0;                                      // Success!
}

int chai::masala::ImStream3::read_non_blocking(void *__restrict__ p, struct timespec *__restrict__ atime) {
  if (!init) return ENOENT;                      // If we aren't initialized, don't even try.
  if (p == nullptr) return EINVAL;               // Try not to segfault.
  std::memcpy(p, im->array.raw, sizeBytes);      // Copy to the buffer.
  if (atime != nullptr) *atime = im->md->atime;  // If requested, copy the time.
  return 0;                                      // Success!
}

void chai::masala::ImStream3::cancel_blocking_read() { ImageStreamIO_sempost(im, semNum); }

int chai::masala::ImStream3::send(void *__restrict__ p, struct timespec atime) {
  if (!init) return ENOENT;                           // If we aren't initialized, don't even try.
  if (p == nullptr) return EINVAL;                    // Try not to segfault.
  im->md->write = MD_WRITE_START;                     // Signal the start of the write.
  std::memcpy(im->array.raw, p, sizeBytes);           // Do the copy.
  im->md->write = MD_WRITE_DONE;                      // Signal that we're done.
  im->md->cnt0++;                                     // Update the image counter.
  clock_gettime(CLOCK_REALTIME, &im->md->writetime);  // Set the write time.
  im->md->atime = atime;                              // Set the acquisition time.
  ImageStreamIO_sempost(im, ALL_SEMAPHORES);          // Let everyone know.
  return 0;                                           // Success!
}
