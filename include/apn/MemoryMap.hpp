/**
 * @project apophnia++
 * @file include/apn/MemoryMap.hpp
 * @author  S Roychowdhury <sroycode AT gmail DOT com>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Read a mmap file
 *
 */

#ifndef _APN_MEMORY_MAP_HPP_
#define _APN_MEMORY_MAP_HPP_
#define APN_MEMORY_MAP_HPP_PROGNO 1007

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem/operations.hpp>         // for real file size
#include <string>

namespace apn {
class MemoryMap {

public:
	typedef unsigned long int Len;
	/**
	 * Constructor: function to initialize from headers
	 *
	 * @param path
	 *   String file input path by addr
	 *
	 * @param num_pages
	 *   unsigned int no of pages
	 *
	 * @return
	 *   none
	 */
	MemoryMap(const std::string& path, unsigned int num_pages):
		path_(path),                                       // file path
		page_start_(0),                                    // starting page
		page_off_set_(0),                                  // starting page offset
		a_page_size_(m_file_.alignment()),                 // a single page size
		mmap_size_(a_page_size_*num_pages),                 // size mmap
		real_file_size_(boost::filesystem::file_size(path)),// real size file
		still_left_(real_file_size_),                      // still left in file
		m_file_(path,                                      // path
		        mmap_size_ > real_file_size_ ?
		        real_file_size_ : mmap_size_,              // map_size
		        0)                                         // initial offset
	{};

	/**
	 * residual: function find left over
	 *
	 * @return
	 *   Len still left to process
	 */
	const Len residual() const {
		return still_left_;
	}

	/**
	 * read_bytes: function to read
	 *
	 * @param buffer
	 *   void* buffer to read in
	 *
	 * @param num_bytes
	 *   Len bytes at a time
	 *
	 * @return
	 *   Len
	 */
	//
	// reads into buffer, the specified num_bytes
	//
	Len read_bytes (void * buffer, Len num_bytes) {

		assert (num_bytes < mmap_size_);// dont read more than a page
		assert ((num_bytes % a_page_size_)==0); // multiple of the page

		if (num_bytes > still_left_)    // don't read more than what
			num_bytes = still_left_;      // we have available

		const Len end_pointer = page_off_set_ + num_bytes;
		if (end_pointer >= mmap_size_) { // repage

			page_start_ += page_off_set_;// record next page start
			m_file_.close ();            // close current page
			m_file_.open (path_, mmap_size_, page_start_); // open next page
			page_off_set_ = 0;           // zero offset
		}
		memcpy(buffer, m_file_.data()+page_off_set_, num_bytes);
		page_off_set_ += num_bytes;    // increment this page off_set
		still_left_ -= num_bytes;      // still left in file

		return num_bytes;              // bytes we read
	}


private:
	std::string path_;     // path to the current file
	Len page_start_;       // the starting page pointer
	Len page_off_set_;     // current file pointer
	Len a_page_size_;      // a single page size
	Len mmap_size_;        // memory map size
	Len real_file_size_;   // real file size
	Len still_left_;       // still left in the file
	boost::iostreams::mapped_file_source m_file_; // current memory map source

};
} // namespace _APN_MEMORY_MAP_HPP_

#endif
