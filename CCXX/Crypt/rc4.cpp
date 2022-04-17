#include <windows.h>
#include <stdio.h>

#define KEY_LENGTH 256

void rc4_init(unsigned char *s, unsigned char *key, unsigned long Len)
{
	int i = 0, j = 0;
	char k[KEY_LENGTH] = { 0 };
	unsigned char tmp = 0;
	for (i = 0; i < KEY_LENGTH; i++)
	{
		s[i] = i;
		k[i] = key[i % Len];
	}
	for (i = 0; i < KEY_LENGTH; i++)
	{
		j = (j + s[i] + k[i]) % KEY_LENGTH;
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
	}
}

void rc4_crypt(unsigned char *s, unsigned char *Data, unsigned long Len)
{
	int i = 0, j = 0, t = 0;
	unsigned long k = 0;
	unsigned char tmp;
	for (k = 0; k < Len; k++)
	{
		i = (i + 1) % KEY_LENGTH;
		j = (j + s[i]) % KEY_LENGTH;
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
		t = (s[i] + s[j]) % KEY_LENGTH;
		Data[k] ^= s[t];
	}
}

int main()
{
	unsigned char s[KEY_LENGTH] = { 0 }; //S-box
	char key[KEY_LENGTH] = { "CEC7CB2B-4C10-471C-8B28-5F0B6BF595A1" };
	char pData[CHAR_MAX] = { "Hello World!" };
	printf("key : %s\n", key);
	printf("raw : %s\n", pData);

	rc4_init(s, (unsigned char *)key, lstrlenA(key));

	rc4_crypt(s, (unsigned char *)pData, lstrlenA(pData));
	printf("encrypt  : %s\n", pData);

	rc4_init(s, (unsigned char *)key, lstrlenA(key)); 
	rc4_crypt(s, (unsigned char *)pData, lstrlenA(pData));
	printf("decrypt  : %s\n", pData);
}