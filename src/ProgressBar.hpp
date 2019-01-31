#pragma once
/*
(The MIT License)

Copyright (c) 2016 Prakhar Srivastav <prakhar@prakhar.me>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cmath>
#include <chrono>
#include <iostream>

class ProgressBar {
private:
	unsigned int ticks = 0u;

	const unsigned int total_ticks = 0u;
	const unsigned int bar_width = 80u;
	const char complete_char = '=';
	const char incomplete_char = ' ';
	const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

public:
	ProgressBar(unsigned int total, unsigned int width, char complete, char incomplete) :
			total_ticks {total}, bar_width {width}, complete_char {complete}, incomplete_char {incomplete} {}

	ProgressBar(unsigned int total, unsigned int width) : total_ticks {total}, bar_width {width} {}

	unsigned int operator++() { return ++ticks; }

	void display() const
	{
		float progress = static_cast<float>(ticks) / static_cast<float>(total_ticks);
		auto pos = static_cast<unsigned short int>(std::lround(static_cast<float>(bar_width) * progress));

		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now-start_time).count();

		std::cout << "[";

		for (unsigned short int i = 0; i < bar_width; ++i) {
			if (i < pos) std::cout << complete_char;
			else if (i == pos) std::cout << ">";
			else std::cout << incomplete_char;
		}
		std::cout << "] " << static_cast<unsigned int>(std::lround(static_cast<float>(bar_width) * progress)) << "% "
				  << static_cast<float>(time_elapsed) / 1000.0f << "s\r";
		std::cout.flush();
	}

	~ProgressBar()
	{
		display();
		std::cout << std::endl;
	}
};