template<typename ResponceType>
struct basic_request
{
	typedef ResponceType response_type;
	http:url url;
};