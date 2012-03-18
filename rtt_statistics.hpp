#pragma once
#include "binary_mode.hpp"

#include <boost/chrono.hpp>
#include <limits>

namespace shoryu
{
struct rtt_header
{
	uint64_t local_timestamp;
	uint64_t rtt_timestamp;
};
bin_ostream& operator<<(bin_ostream& os, const rtt_header& rtt)
{
	os << rtt.local_timestamp << rtt.rtt_timestamp;
	return os;
}
bin_istream& operator>>(bin_istream& is, rtt_header& rtt)
{
	is >> rtt.local_timestamp >> rtt.rtt_timestamp;
	return is;
}
// Round trip time statistics
struct rtt_statistics
{
	typedef rtt_header header_type;

	rtt_statistics() :
		_min(std::numeric_limits<decltype(_min)>::max()),
		_max(std::numeric_limits<decltype(_max)>::min()),
		_avg(0)
	{
	}
	header_type get_header() const
	{
		header_type header;
		header.local_timestamp = nanosec_clock();
		header.rtt_timestamp = _remote_ts + (header.local_timestamp - _recv_ts);
		return header;
	}
	void parse_header(const header_type& header)
	{
		_recv_ts = nanosec_clock();
		_remote_ts = header.local_timestamp;
		uint64_t value = _recv_ts - header.rtt_timestamp;
		if(_min > _max)
			_avg = value;
		else
			_avg = (3*value + 7*_avg)/10;

		if(_min > value || _min < 0)
			_min = value;
		if(_max < value || _max < 0)
			_max = value;
	}

	uint64_t avg_rtt() const { return _avg; };
	uint64_t min_rtt() const { return _min; };
	uint64_t max_rtt() const { return _max; };
protected:
	uint64_t nanosec_clock() const
	{
		using namespace boost::chrono;
		return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}
	uint64_t _remote_ts;
	uint64_t _recv_ts;
	uint64_t _avg;
	uint64_t _min;
	uint64_t _max;
};

bin_ostream& operator<<(bin_ostream& os, const rtt_statistics& app)
{
	os << app.get_header();
	return os;
}
bin_istream& operator>>(bin_istream& is, rtt_statistics& app)
{
	rtt_statistics::header_type header;
	is >> header;
	app.parse_header(header);
	return is;
}

}