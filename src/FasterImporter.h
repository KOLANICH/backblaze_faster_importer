#pragma once

enum{
	offsetDay=15340u,
	secondsInHour=3600u,
	secondsInDay=secondsInHour*24u
};


typedef uint32_t PackedRowidT;
typedef uint32_t DriveIdT;
typedef uint32_t ModelIdT;
typedef uint8_t BrandIdT;

typedef uint32_t AttrT;

struct Drive{
	DriveIdT id = 0;
};

struct Brand{
	std::regex rx;
	BrandIdT id;
};

struct Model{
	ModelIdT id;
	int64_t capacity_bytes;
	BrandIdT brandId;
};

/* Do not pack rowids using bit fields, despite __attribute__((packed)); G++ will pack them as (the example is initialized with a De Bruijn sequence)

sizeof(PackedRowidT2) 8
bytes ruler								|--------|--- -----|							|----- ---|----- ---|--------|--------|--------|--------|
a.components.offsettedDay (le)			 01101011 000 01110 a.components.driveId (le)	 00011 111 00101 001 00000000 00000000
whole union (read as bytes, le)			 01101011 000 01110								 00011 111 00101 001 00000000 00000000 00000000 00000000

union (uint32_t-accessible part, le)	 01101011 000 01110								 00011 111 00101 001
bits belongings							 ++++++++ ~~~ +++++								 76543 210 fedcb a98 ~~~~~ihg ~~~~~~~~ ~~~~~~~~ ~~~~~~~~

manually packed (le)					 01101011 111 01110								 001 00011 00000 101
bits belongings							 ++++++++ 210 +++++								 a98 76543 ihgfe dcb

where + marks bits belonging to offsettedDay, digits mark bits belonging to driveId, ~ marks unused bits
*/

constexpr PackedRowidT packRowid(uint32_t driveId, uint32_t day){
	return driveId << 13 | (day - offsetDay);
}