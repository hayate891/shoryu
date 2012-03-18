#pragma once

#include <boost/static_assert.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <boost/detail/endian.hpp>

#include <iostream>
#include <exception>

// TODO: split definition/declaration, move details to separate headers
// TODO: support more types (for instance, std::string, static arrays, vectors, bitsets)
namespace shoryu
{
BOOST_STATIC_ASSERT(CHAR_BIT==8);

using boost::uint8_t;
using boost::int8_t;
using boost::uint16_t;
using boost::int16_t;
using boost::uint32_t;
using boost::int32_t;
using boost::uint64_t;
using boost::int64_t;


class binary_mode_exception : 
    public virtual std::exception
{
public:
    typedef enum {
        no_exception,       // initialized without code
        other_exception,    // any excepton not listed below
        input_stream_error, // error on input stream
        output_stream_error // error on output stream
    } exception_code;

    exception_code code;
    binary_mode_exception(exception_code c = no_exception, const char* msg = "") : code(c), m_msg(msg) {}
	~binary_mode_exception() {}
	virtual const char *what() const throw() { return m_msg.c_str(); }
protected:
	std::string m_msg;
};

namespace detail
{
inline int16_t reorder(int16_t x)
{
	return (static_cast<uint16_t>(x) << 8)
		| (static_cast<uint16_t>(x) >> 8);
}

inline int32_t reorder(int32_t x)
{
	uint32_t step16 = static_cast<uint32_t>(x) << 16 | static_cast<uint32_t>(x) >> 16;
	return
		((static_cast<uint32_t>(step16) << 8) & 0xff00ff00)
		| ((static_cast<uint32_t>(step16) >> 8) & 0x00ff00ff);
}

inline int64_t reorder(int64_t x)
{
	uint64_t step32, step16;
	step32 = static_cast<uint64_t>(x) << 32 | static_cast<uint64_t>(x) >> 32;
	step16 = (step32 & 0x0000FFFF0000FFFF) << 16
		| (step32 & 0xFFFF0000FFFF0000) >> 16;
	return static_cast<int64_t>((step16 & 0x00FF00FF00FF00FF) << 8
		| (step16 & 0xFF00FF00FF00FF00) >> 8);
}

inline uint16_t reorder(uint16_t x)
{
	return (x << 8)
		| (x >> 8);
}

inline uint32_t reorder(uint32_t x)
{
	uint32_t step16;
	step16 = x << 16 | x >> 16;
	return
		((step16 << 8) & 0xff00ff00)
		| ((step16 >> 8) & 0x00ff00ff);
}

inline uint64_t reorder(uint64_t x)
{
	uint64_t step32, step16;
	step32 = x << 32 | x >> 32;
	step16 = (step32 & 0x0000FFFF0000FFFF) << 16
		| (step32 & 0xFFFF0000FFFF0000) >> 16;
	return (step16 & 0x00FF00FF00FF00FF) << 8
		| (step16 & 0xFF00FF00FF00FF00) >> 8;
}

struct bin_ostream
{
	bin_ostream(std::ostream& out_) : out(out_) {}
	bin_ostream& write(const char* data, std::streamsize size)
	{
		BOOST_ASSERT(data != 0);
		if(out.fail())
			boost::throw_exception(binary_mode_exception(binary_mode_exception::input_stream_error));
		
		out.write(data, size);

		return (*this);
	}
private:
	std::ostream& out;
};

struct bin_istream
{
	bin_istream(std::istream& in_) : in(in_) {}
	bin_istream& read(char* data, std::streamsize size)
	{
		BOOST_ASSERT(data != 0);
		if(in.fail())
			boost::throw_exception(binary_mode_exception(binary_mode_exception::input_stream_error));
		
		in.read(data, size);

		return (*this);
	}
private:
	std::istream& in;
};

struct bin_iostream : bin_ostream, bin_istream
{
	bin_iostream(std::iostream& ios) : bin_ostream(ios), bin_istream(ios) {}
};

#ifdef BOOST_LITTLE_ENDIAN
	const bool needs_reordering = true;
#elif
	const bool needs_reordering = false;
#endif

#define DECLARE_INTEGER_INSERTION_OP(integer_type) \
bin_ostream& operator<<(bin_ostream& oa, integer_type val) \
{ \
	if(needs_reordering && sizeof(integer_type) != 1 ) val = reorder(val); \
	oa.write((const char*)&val, sizeof(integer_type)); \
	return oa; \
}
DECLARE_INTEGER_INSERTION_OP(uint8_t);
DECLARE_INTEGER_INSERTION_OP(int8_t);
DECLARE_INTEGER_INSERTION_OP(uint16_t);
DECLARE_INTEGER_INSERTION_OP(int16_t);
DECLARE_INTEGER_INSERTION_OP(uint32_t);
DECLARE_INTEGER_INSERTION_OP(int32_t);
DECLARE_INTEGER_INSERTION_OP(uint64_t);
DECLARE_INTEGER_INSERTION_OP(int64_t);

#define DECLARE_INTEGER_EXTRACTION_OP(integer_type) \
bin_istream& operator>>(bin_istream& ia, integer_type& val) \
{ \
	ia.read((char*)&val, sizeof(integer_type)); \
	if(needs_reordering && sizeof(integer_type) != 1) val = reorder(val); \
	return ia; \
}
DECLARE_INTEGER_EXTRACTION_OP(uint8_t);
DECLARE_INTEGER_EXTRACTION_OP(int8_t);
DECLARE_INTEGER_EXTRACTION_OP(uint16_t);
DECLARE_INTEGER_EXTRACTION_OP(int16_t);
DECLARE_INTEGER_EXTRACTION_OP(uint32_t);
DECLARE_INTEGER_EXTRACTION_OP(int32_t);
DECLARE_INTEGER_EXTRACTION_OP(uint64_t);
DECLARE_INTEGER_EXTRACTION_OP(int64_t);

}

using detail::bin_iostream;
using detail::bin_ostream;
using detail::bin_istream;

bin_iostream binary_mode(std::iostream& ios)
{
	return bin_iostream(ios);
}
bin_ostream binary_mode(std::ostream& os)
{
	return bin_ostream(os);
}
bin_istream binary_mode(std::istream& is)
{
	return bin_istream(is);
}

}