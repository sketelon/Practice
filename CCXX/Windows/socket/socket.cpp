/*

<必须包含头文件及lib>


```
#include <winsock2.h>
#pragma comment (lib, "Ws2_32.lib")
```


<常用相关宏>

INADDR_ANY	（表示可以接受任何链接，通常是 server）
INADDR_LOOPBACK  （127.0.0.1）


<相关函数>
inet_addr: 将字符串ip地址转化为长整形, 不过现在已 deprecated, 推荐使用 inet_pton 或 InetPton. 转化后为 Big Endian(网络字节顺序)



*/


typedef struct  _MYULONG
{
	union {
		struct {
			ULONG u1 : 8;
			ULONG u2 : 8;
			ULONG u3 : 8;
			ULONG u4 : 8;
		} s_u;
		ULONG u;
	} un_ulong;
}MYULONG, *PMYULONG;

typedef struct  _MYUSHORT
{
	union {
		struct {
			USHORT us1 : 8;
			USHORT us2 : 8;
		} s_us;
		USHORT us;
	} un_us;
}MYUSHORT, * PMYUSHORT;

ULONG ulong_byte_swap(ULONG i)
{
	MYULONG tmp;
	MYULONG u;
	tmp.un_ulong.u = i;
	// 1234 -> 4321
	u.un_ulong.s_u.u1 = tmp.un_ulong.s_u.u4;
	u.un_ulong.s_u.u2 = tmp.un_ulong.s_u.u3;
	u.un_ulong.s_u.u3 = tmp.un_ulong.s_u.u2;
	u.un_ulong.s_u.u4 = tmp.un_ulong.s_u.u1;

	return u.un_ulong.u;
}

USHORT ushort_byte_swap(USHORT i)
{
	MYUSHORT tmp;
	MYUSHORT us;
	tmp.un_us.us = i;

	us.un_us.s_us.us1 = tmp.un_us.s_us.us2;
	us.un_us.s_us.us2 = tmp.un_us.s_us.us1;

	return us.un_us.us;
}
