#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <stdint.h>
#include <sys/types.h>

#define __bswap_16(x) ((uint16_t)((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8)))
#define __bswap_32(x) ((uint32_t)((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | \
    (((x) & 0x0000ff00) << 8)  | (((x) & 0x000000ff) << 24)))

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htobe16(x) __bswap_16(x)
#define htole16(x) (x)
#define be16toh(x) __bswap_16(x)
#define le16toh(x) (x)
#define htobe32(x) __bswap_32(x)
#define htole32(x) (x)
#define be32toh(x) __bswap_32(x)
#define le32toh(x) (x)
#else
#define htobe16(x) (x)
#define htole16(x) __bswap_16(x)
#define be16toh(x) (x)
#define le16toh(x) __bswap_16(x)
#define htobe32(x) (x)
#define htole32(x) __bswap_32(x)
#define be32toh(x) (x)
#define le32toh(x) __bswap_32(x)
#endif

#endif
