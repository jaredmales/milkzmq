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

#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "ims3.h"

#define DIM 32

using namespace std::chrono_literals;

static std::string rand_str(int len) {
  static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, sizeof(alphabet) - 2);
  std::string retval;
  for (int i = 0; i < len; i++) {
    retval += alphabet[dis(gen)];
  }
  return retval;
}

static void fill(std::array<float, DIM * DIM>& data) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis;
  for (int i = 0; i < data.size(); i++) {
    data[i] = dis(gen);
  }
}

int main(int argc, char* argv[]) {
  // Open a randomly named image stream.
  auto name = rand_str(16);
  auto is = new chai::masala::ImStream3(name, DIM, DIM, _DATATYPE_FLOAT);
  #if (__cpp_lib_format >= 202207L)
  std::cout << std::format("New image stream is named {}.", name) << std::endl;
  #endif

  // Make some data and send it at one hz.
  std::array<float, DIM * DIM> arr;
  while (true) {
    fill(arr);
    is->send(arr.data());
    std::this_thread::sleep_for(1s);
  }
  return 0;
}
