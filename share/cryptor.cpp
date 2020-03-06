#include "cryptor.h"

#pragma region hex table for encryption and decryption
uint32_t fb::cryptor::HEX_TABLE[10][0x100] = 
{
{ 0x00000000, 0x01010101, 0x02020202, 0x03030303, 0x04040404, 0x05050505, 0x06060606, 0x07070707, 0x08080808, 0x09090909, 0x0A0A0A0A, 0x0B0B0B0B, 0x0C0C0C0C, 0x0D0D0D0D, 0x0E0E0E0E, 0x0F0F0F0F, 0x10101010, 0x11111111, 0x12121212, 0x13131313, 0x14141414, 0x15151515, 0x16161616, 0x17171717, 0x18181818, 0x19191919, 0x1A1A1A1A, 0x1B1B1B1B, 0x1C1C1C1C, 0x1D1D1D1D, 0x1E1E1E1E, 0x1F1F1F1F, 0x20202020, 0x21212121, 0x22222222, 0x23232323, 0x24242424, 0x25252525, 0x26262626, 0x27272727, 0x28282828, 0x29292929, 0x2A2A2A2A, 0x2B2B2B2B, 0x2C2C2C2C, 0x2D2D2D2D, 0x2E2E2E2E, 0x2F2F2F2F, 0x30303030, 0x31313131, 0x32323232, 0x33333333, 0x34343434, 0x35353535, 0x36363636, 0x37373737, 0x38383838, 0x39393939, 0x3A3A3A3A, 0x3B3B3B3B, 0x3C3C3C3C, 0x3D3D3D3D, 0x3E3E3E3E, 0x3F3F3F3F, 0x40404040, 0x41414141, 0x42424242, 0x43434343, 0x44444444, 0x45454545, 0x46464646, 0x47474747, 0x48484848, 0x49494949, 0x4A4A4A4A, 0x4B4B4B4B, 0x4C4C4C4C, 0x4D4D4D4D, 0x4E4E4E4E, 0x4F4F4F4F, 0x50505050, 0x51515151, 0x52525252, 0x53535353, 0x54545454, 0x55555555, 0x56565656, 0x57575757, 0x58585858, 0x59595959, 0x5A5A5A5A, 0x5B5B5B5B, 0x5C5C5C5C, 0x5D5D5D5D, 0x5E5E5E5E, 0x5F5F5F5F, 0x60606060, 0x61616161, 0x62626262, 0x63636363, 0x64646464, 0x65656565, 0x66666666, 0x67676767, 0x68686868, 0x69696969, 0x6A6A6A6A, 0x6B6B6B6B, 0x6C6C6C6C, 0x6D6D6D6D, 0x6E6E6E6E, 0x6F6F6F6F, 0x70707070, 0x71717171, 0x72727272, 0x73737373, 0x74747474, 0x75757575, 0x76767676, 0x77777777, 0x78787878, 0x79797979, 0x7A7A7A7A, 0x7B7B7B7B, 0x7C7C7C7C, 0x7D7D7D7D, 0x7E7E7E7E, 0x7F7F7F7F, 0x80808080, 0x81818181, 0x82828282, 0x83838383, 0x84848484, 0x85858585, 0x86868686, 0x87878787, 0x88888888, 0x89898989, 0x8A8A8A8A, 0x8B8B8B8B, 0x8C8C8C8C, 0x8D8D8D8D, 0x8E8E8E8E, 0x8F8F8F8F, 0x90909090, 0x91919191, 0x92929292, 0x93939393, 0x94949494, 0x95959595, 0x96969696, 0x97979797, 0x98989898, 0x99999999, 0x9A9A9A9A, 0x9B9B9B9B, 0x9C9C9C9C, 0x9D9D9D9D, 0x9E9E9E9E, 0x9F9F9F9F, 0xA0A0A0A0, 0xA1A1A1A1, 0xA2A2A2A2, 0xA3A3A3A3, 0xA4A4A4A4, 0xA5A5A5A5, 0xA6A6A6A6, 0xA7A7A7A7, 0xA8A8A8A8, 0xA9A9A9A9, 0xAAAAAAAA, 0xABABABAB, 0xACACACAC, 0xADADADAD, 0xAEAEAEAE, 0xAFAFAFAF, 0xB0B0B0B0, 0xB1B1B1B1, 0xB2B2B2B2, 0xB3B3B3B3, 0xB4B4B4B4, 0xB5B5B5B5, 0xB6B6B6B6, 0xB7B7B7B7, 0xB8B8B8B8, 0xB9B9B9B9, 0xBABABABA, 0xBBBBBBBB, 0xBCBCBCBC, 0xBDBDBDBD, 0xBEBEBEBE, 0xBFBFBFBF, 0xC0C0C0C0, 0xC1C1C1C1, 0xC2C2C2C2, 0xC3C3C3C3, 0xC4C4C4C4, 0xC5C5C5C5, 0xC6C6C6C6, 0xC7C7C7C7, 0xC8C8C8C8, 0xC9C9C9C9, 0xCACACACA, 0xCBCBCBCB, 0xCCCCCCCC, 0xCDCDCDCD, 0xCECECECE, 0xCFCFCFCF, 0xD0D0D0D0, 0xD1D1D1D1, 0xD2D2D2D2, 0xD3D3D3D3, 0xD4D4D4D4, 0xD5D5D5D5, 0xD6D6D6D6, 0xD7D7D7D7, 0xD8D8D8D8, 0xD9D9D9D9, 0xDADADADA, 0xDBDBDBDB, 0xDCDCDCDC, 0xDDDDDDDD, 0xDEDEDEDE, 0xDFDFDFDF, 0xE0E0E0E0, 0xE1E1E1E1, 0xE2E2E2E2, 0xE3E3E3E3, 0xE4E4E4E4, 0xE5E5E5E5, 0xE6E6E6E6, 0xE7E7E7E7, 0xE8E8E8E8, 0xE9E9E9E9, 0xEAEAEAEA, 0xEBEBEBEB, 0xECECECEC, 0xEDEDEDED, 0xEEEEEEEE, 0xEFEFEFEF, 0xF0F0F0F0, 0xF1F1F1F1, 0xF2F2F2F2, 0xF3F3F3F3, 0xF4F4F4F4, 0xF5F5F5F5, 0xF6F6F6F6, 0xF7F7F7F7, 0xF8F8F8F8, 0xF9F9F9F9, 0xFAFAFAFA, 0xFBFBFBFB, 0xFCFCFCFC, 0xFDFDFDFD, 0xFEFEFEFE, 0xFFFFFFFF },
{ 0x80808080, 0x7F7F7F7F, 0x81818181, 0x7E7E7E7E, 0x82828282, 0x7D7D7D7D, 0x83838383, 0x7C7C7C7C, 0x84848484, 0x7B7B7B7B, 0x85858585, 0x7A7A7A7A, 0x86868686, 0x79797979, 0x87878787, 0x78787878, 0x88888888, 0x77777777, 0x89898989, 0x76767676, 0x8A8A8A8A, 0x75757575, 0x8B8B8B8B, 0x74747474, 0x8C8C8C8C, 0x73737373, 0x8D8D8D8D, 0x72727272, 0x8E8E8E8E, 0x71717171, 0x8F8F8F8F, 0x70707070, 0x90909090, 0x6F6F6F6F, 0x91919191, 0x6E6E6E6E, 0x92929292, 0x6D6D6D6D, 0x93939393, 0x6C6C6C6C, 0x94949494, 0x6B6B6B6B, 0x95959595, 0x6A6A6A6A, 0x96969696, 0x69696969, 0x97979797, 0x68686868, 0x98989898, 0x67676767, 0x99999999, 0x66666666, 0x9A9A9A9A, 0x65656565, 0x9B9B9B9B, 0x64646464, 0x9C9C9C9C, 0x63636363, 0x9D9D9D9D, 0x62626262, 0x9E9E9E9E, 0x61616161, 0x9F9F9F9F, 0x60606060, 0xA0A0A0A0, 0x5F5F5F5F, 0xA1A1A1A1, 0x5E5E5E5E, 0xA2A2A2A2, 0x5D5D5D5D, 0xA3A3A3A3, 0x5C5C5C5C, 0xA4A4A4A4, 0x5B5B5B5B, 0xA5A5A5A5, 0x5A5A5A5A, 0xA6A6A6A6, 0x59595959, 0xA7A7A7A7, 0x58585858, 0xA8A8A8A8, 0x57575757, 0xA9A9A9A9, 0x56565656, 0xAAAAAAAA, 0x55555555, 0xABABABAB, 0x54545454, 0xACACACAC, 0x53535353, 0xADADADAD, 0x52525252, 0xAEAEAEAE, 0x51515151, 0xAFAFAFAF, 0x50505050, 0xB0B0B0B0, 0x4F4F4F4F, 0xB1B1B1B1, 0x4E4E4E4E, 0xB2B2B2B2, 0x4D4D4D4D, 0xB3B3B3B3, 0x4C4C4C4C, 0xB4B4B4B4, 0x4B4B4B4B, 0xB5B5B5B5, 0x4A4A4A4A, 0xB6B6B6B6, 0x49494949, 0xB7B7B7B7, 0x48484848, 0xB8B8B8B8, 0x47474747, 0xB9B9B9B9, 0x46464646, 0xBABABABA, 0x45454545, 0xBBBBBBBB, 0x44444444, 0xBCBCBCBC, 0x43434343, 0xBDBDBDBD, 0x42424242, 0xBEBEBEBE, 0x41414141, 0xBFBFBFBF, 0x40404040, 0xC0C0C0C0, 0x3F3F3F3F, 0xC1C1C1C1, 0x3E3E3E3E, 0xC2C2C2C2, 0x3D3D3D3D, 0xC3C3C3C3, 0x3C3C3C3C, 0xC4C4C4C4, 0x3B3B3B3B, 0xC5C5C5C5, 0x3A3A3A3A, 0xC6C6C6C6, 0x39393939, 0xC7C7C7C7, 0x38383838, 0xC8C8C8C8, 0x37373737, 0xC9C9C9C9, 0x36363636, 0xCACACACA, 0x35353535, 0xCBCBCBCB, 0x34343434, 0xCCCCCCCC, 0x33333333, 0xCDCDCDCD, 0x32323232, 0xCECECECE, 0x31313131, 0xCFCFCFCF, 0x30303030, 0xD0D0D0D0, 0x2F2F2F2F, 0xD1D1D1D1, 0x2E2E2E2E, 0xD2D2D2D2, 0x2D2D2D2D, 0xD3D3D3D3, 0x2C2C2C2C, 0xD4D4D4D4, 0x2B2B2B2B, 0xD5D5D5D5, 0x2A2A2A2A, 0xD6D6D6D6, 0x29292929, 0xD7D7D7D7, 0x28282828, 0xD8D8D8D8, 0x27272727, 0xD9D9D9D9, 0x26262626, 0xDADADADA, 0x25252525, 0xDBDBDBDB, 0x24242424, 0xDCDCDCDC, 0x23232323, 0xDDDDDDDD, 0x22222222, 0xDEDEDEDE, 0x21212121, 0xDFDFDFDF, 0x20202020, 0xE0E0E0E0, 0x1F1F1F1F, 0xE1E1E1E1, 0x1E1E1E1E, 0xE2E2E2E2, 0x1D1D1D1D, 0xE3E3E3E3, 0x1C1C1C1C, 0xE4E4E4E4, 0x1B1B1B1B, 0xE5E5E5E5, 0x1A1A1A1A, 0xE6E6E6E6, 0x19191919, 0xE7E7E7E7, 0x18181818, 0xE8E8E8E8, 0x17171717, 0xE9E9E9E9, 0x16161616, 0xEAEAEAEA, 0x15151515, 0xEBEBEBEB, 0x14141414, 0xECECECEC, 0x13131313, 0xEDEDEDED, 0x12121212, 0xEEEEEEEE, 0x11111111, 0xEFEFEFEF, 0x10101010, 0xF0F0F0F0, 0x0F0F0F0F, 0xF1F1F1F1, 0x0E0E0E0E, 0xF2F2F2F2, 0x0D0D0D0D, 0xF3F3F3F3, 0x0C0C0C0C, 0xF4F4F4F4, 0x0B0B0B0B, 0xF5F5F5F5, 0x0A0A0A0A, 0xF6F6F6F6, 0x09090909, 0xF7F7F7F7, 0x08080808, 0xF8F8F8F8, 0x07070707, 0xF9F9F9F9, 0x06060606, 0xFAFAFAFA, 0x05050505, 0xFBFBFBFB, 0x04040404, 0xFCFCFCFC, 0x03030303, 0xFDFDFDFD, 0x02020202, 0xFEFEFEFE, 0x01010101, 0xFFFFFFFF, 0x00000000 },
{ 0xFFFFFFFF, 0xFEFEFEFE, 0xFDFDFDFD, 0xFCFCFCFC, 0xFBFBFBFB, 0xFAFAFAFA, 0xF9F9F9F9, 0xF8F8F8F8, 0xF7F7F7F7, 0xF6F6F6F6, 0xF5F5F5F5, 0xF4F4F4F4, 0xF3F3F3F3, 0xF2F2F2F2, 0xF1F1F1F1, 0xF0F0F0F0, 0xEFEFEFEF, 0xEEEEEEEE, 0xEDEDEDED, 0xECECECEC, 0xEBEBEBEB, 0xEAEAEAEA, 0xE9E9E9E9, 0xE8E8E8E8, 0xE7E7E7E7, 0xE6E6E6E6, 0xE5E5E5E5, 0xE4E4E4E4, 0xE3E3E3E3, 0xE2E2E2E2, 0xE1E1E1E1, 0xE0E0E0E0, 0xDFDFDFDF, 0xDEDEDEDE, 0xDDDDDDDD, 0xDCDCDCDC, 0xDBDBDBDB, 0xDADADADA, 0xD9D9D9D9, 0xD8D8D8D8, 0xD7D7D7D7, 0xD6D6D6D6, 0xD5D5D5D5, 0xD4D4D4D4, 0xD3D3D3D3, 0xD2D2D2D2, 0xD1D1D1D1, 0xD0D0D0D0, 0xCFCFCFCF, 0xCECECECE, 0xCDCDCDCD, 0xCCCCCCCC, 0xCBCBCBCB, 0xCACACACA, 0xC9C9C9C9, 0xC8C8C8C8, 0xC7C7C7C7, 0xC6C6C6C6, 0xC5C5C5C5, 0xC4C4C4C4, 0xC3C3C3C3, 0xC2C2C2C2, 0xC1C1C1C1, 0xC0C0C0C0, 0xBFBFBFBF, 0xBEBEBEBE, 0xBDBDBDBD, 0xBCBCBCBC, 0xBBBBBBBB, 0xBABABABA, 0xB9B9B9B9, 0xB8B8B8B8, 0xB7B7B7B7, 0xB6B6B6B6, 0xB5B5B5B5, 0xB4B4B4B4, 0xB3B3B3B3, 0xB2B2B2B2, 0xB1B1B1B1, 0xB0B0B0B0, 0xAFAFAFAF, 0xAEAEAEAE, 0xADADADAD, 0xACACACAC, 0xABABABAB, 0xAAAAAAAA, 0xA9A9A9A9, 0xA8A8A8A8, 0xA7A7A7A7, 0xA6A6A6A6, 0xA5A5A5A5, 0xA4A4A4A4, 0xA3A3A3A3, 0xA2A2A2A2, 0xA1A1A1A1, 0xA0A0A0A0, 0x9F9F9F9F, 0x9E9E9E9E, 0x9D9D9D9D, 0x9C9C9C9C, 0x9B9B9B9B, 0x9A9A9A9A, 0x99999999, 0x98989898, 0x97979797, 0x96969696, 0x95959595, 0x94949494, 0x93939393, 0x92929292, 0x91919191, 0x90909090, 0x8F8F8F8F, 0x8E8E8E8E, 0x8D8D8D8D, 0x8C8C8C8C, 0x8B8B8B8B, 0x8A8A8A8A, 0x89898989, 0x88888888, 0x87878787, 0x86868686, 0x85858585, 0x84848484, 0x83838383, 0x82828282, 0x81818181, 0x80808080, 0x7F7F7F7F, 0x7E7E7E7E, 0x7D7D7D7D, 0x7C7C7C7C, 0x7B7B7B7B, 0x7A7A7A7A, 0x79797979, 0x78787878, 0x77777777, 0x76767676, 0x75757575, 0x74747474, 0x73737373, 0x72727272, 0x71717171, 0x70707070, 0x6F6F6F6F, 0x6E6E6E6E, 0x6D6D6D6D, 0x6C6C6C6C, 0x6B6B6B6B, 0x6A6A6A6A, 0x69696969, 0x68686868, 0x67676767, 0x66666666, 0x65656565, 0x64646464, 0x63636363, 0x62626262, 0x61616161, 0x60606060, 0x5F5F5F5F, 0x5E5E5E5E, 0x5D5D5D5D, 0x5C5C5C5C, 0x5B5B5B5B, 0x5A5A5A5A, 0x59595959, 0x58585858, 0x57575757, 0x56565656, 0x55555555, 0x54545454, 0x53535353, 0x52525252, 0x51515151, 0x50505050, 0x4F4F4F4F, 0x4E4E4E4E, 0x4D4D4D4D, 0x4C4C4C4C, 0x4B4B4B4B, 0x4A4A4A4A, 0x49494949, 0x48484848, 0x47474747, 0x46464646, 0x45454545, 0x44444444, 0x43434343, 0x42424242, 0x41414141, 0x40404040, 0x3F3F3F3F, 0x3E3E3E3E, 0x3D3D3D3D, 0x3C3C3C3C, 0x3B3B3B3B, 0x3A3A3A3A, 0x39393939, 0x38383838, 0x37373737, 0x36363636, 0x35353535, 0x34343434, 0x33333333, 0x32323232, 0x31313131, 0x30303030, 0x2F2F2F2F, 0x2E2E2E2E, 0x2D2D2D2D, 0x2C2C2C2C, 0x2B2B2B2B, 0x2A2A2A2A, 0x29292929, 0x28282828, 0x27272727, 0x26262626, 0x25252525, 0x24242424, 0x23232323, 0x22222222, 0x21212121, 0x20202020, 0x1F1F1F1F, 0x1E1E1E1E, 0x1D1D1D1D, 0x1C1C1C1C, 0x1B1B1B1B, 0x1A1A1A1A, 0x19191919, 0x18181818, 0x17171717, 0x16161616, 0x15151515, 0x14141414, 0x13131313, 0x12121212, 0x11111111, 0x10101010, 0x0F0F0F0F, 0x0E0E0E0E, 0x0D0D0D0D, 0x0C0C0C0C, 0x0B0B0B0B, 0x0A0A0A0A, 0x09090909, 0x08080808, 0x07070707, 0x06060606, 0x05050505, 0x04040404, 0x03030303, 0x02020202, 0x01010101, 0x00000000 },
{ 0xFFFFFFFF, 0x01010101, 0xFEFEFEFE, 0x02020202, 0xFDFDFDFD, 0x03030303, 0xFCFCFCFC, 0x04040404, 0xFBFBFBFB, 0x05050505, 0xFAFAFAFA, 0x06060606, 0xF9F9F9F9, 0x07070707, 0xF8F8F8F8, 0x08080808, 0xF7F7F7F7, 0x09090909, 0xF6F6F6F6, 0x0A0A0A0A, 0xF5F5F5F5, 0x0B0B0B0B, 0xF4F4F4F4, 0x0C0C0C0C, 0xF3F3F3F3, 0x0D0D0D0D, 0xF2F2F2F2, 0x0E0E0E0E, 0xF1F1F1F1, 0x0F0F0F0F, 0xF0F0F0F0, 0x10101010, 0xEFEFEFEF, 0x11111111, 0xEEEEEEEE, 0x12121212, 0xEDEDEDED, 0x13131313, 0xECECECEC, 0x14141414, 0xEBEBEBEB, 0x15151515, 0xEAEAEAEA, 0x16161616, 0xE9E9E9E9, 0x17171717, 0xE8E8E8E8, 0x18181818, 0xE7E7E7E7, 0x19191919, 0xE6E6E6E6, 0x1A1A1A1A, 0xE5E5E5E5, 0x1B1B1B1B, 0xE4E4E4E4, 0x1C1C1C1C, 0xE3E3E3E3, 0x1D1D1D1D, 0xE2E2E2E2, 0x1E1E1E1E, 0xE1E1E1E1, 0x1F1F1F1F, 0xE0E0E0E0, 0x20202020, 0xDFDFDFDF, 0x21212121, 0xDEDEDEDE, 0x22222222, 0xDDDDDDDD, 0x23232323, 0xDCDCDCDC, 0x24242424, 0xDBDBDBDB, 0x25252525, 0xDADADADA, 0x26262626, 0xD9D9D9D9, 0x27272727, 0xD8D8D8D8, 0x28282828, 0xD7D7D7D7, 0x29292929, 0xD6D6D6D6, 0x2A2A2A2A, 0xD5D5D5D5, 0x2B2B2B2B, 0xD4D4D4D4, 0x2C2C2C2C, 0xD3D3D3D3, 0x2D2D2D2D, 0xD2D2D2D2, 0x2E2E2E2E, 0xD1D1D1D1, 0x2F2F2F2F, 0xD0D0D0D0, 0x30303030, 0xCFCFCFCF, 0x31313131, 0xCECECECE, 0x32323232, 0xCDCDCDCD, 0x33333333, 0xCCCCCCCC, 0x34343434, 0xCBCBCBCB, 0x35353535, 0xCACACACA, 0x36363636, 0xC9C9C9C9, 0x37373737, 0xC8C8C8C8, 0x38383838, 0xC7C7C7C7, 0x39393939, 0xC6C6C6C6, 0x3A3A3A3A, 0xC5C5C5C5, 0x3B3B3B3B, 0xC4C4C4C4, 0x3C3C3C3C, 0xC3C3C3C3, 0x3D3D3D3D, 0xC2C2C2C2, 0x3E3E3E3E, 0xC1C1C1C1, 0x3F3F3F3F, 0xC0C0C0C0, 0x40404040, 0xBFBFBFBF, 0x41414141, 0xBEBEBEBE, 0x42424242, 0xBDBDBDBD, 0x43434343, 0xBCBCBCBC, 0x44444444, 0xBBBBBBBB, 0x45454545, 0xBABABABA, 0x46464646, 0xB9B9B9B9, 0x47474747, 0xB8B8B8B8, 0x48484848, 0xB7B7B7B7, 0x49494949, 0xB6B6B6B6, 0x4A4A4A4A, 0xB5B5B5B5, 0x4B4B4B4B, 0xB4B4B4B4, 0x4C4C4C4C, 0xB3B3B3B3, 0x4D4D4D4D, 0xB2B2B2B2, 0x4E4E4E4E, 0xB1B1B1B1, 0x4F4F4F4F, 0xB0B0B0B0, 0x50505050, 0xAFAFAFAF, 0x51515151, 0xAEAEAEAE, 0x52525252, 0xADADADAD, 0x53535353, 0xACACACAC, 0x54545454, 0xABABABAB, 0x55555555, 0xAAAAAAAA, 0x56565656, 0xA9A9A9A9, 0x57575757, 0xA8A8A8A8, 0x58585858, 0xA7A7A7A7, 0x59595959, 0xA6A6A6A6, 0x5A5A5A5A, 0xA5A5A5A5, 0x5B5B5B5B, 0xA4A4A4A4, 0x5C5C5C5C, 0xA3A3A3A3, 0x5D5D5D5D, 0xA2A2A2A2, 0x5E5E5E5E, 0xA1A1A1A1, 0x5F5F5F5F, 0xA0A0A0A0, 0x60606060, 0x9F9F9F9F, 0x61616161, 0x9E9E9E9E, 0x62626262, 0x9D9D9D9D, 0x63636363, 0x9C9C9C9C, 0x64646464, 0x9B9B9B9B, 0x65656565, 0x9A9A9A9A, 0x66666666, 0x99999999, 0x67676767, 0x98989898, 0x68686868, 0x97979797, 0x69696969, 0x96969696, 0x6A6A6A6A, 0x95959595, 0x6B6B6B6B, 0x94949494, 0x6C6C6C6C, 0x93939393, 0x6D6D6D6D, 0x92929292, 0x6E6E6E6E, 0x91919191, 0x6F6F6F6F, 0x90909090, 0x70707070, 0x8F8F8F8F, 0x71717171, 0x8E8E8E8E, 0x72727272, 0x8D8D8D8D, 0x73737373, 0x8C8C8C8C, 0x74747474, 0x8B8B8B8B, 0x75757575, 0x8A8A8A8A, 0x76767676, 0x89898989, 0x77777777, 0x88888888, 0x78787878, 0x87878787, 0x79797979, 0x86868686, 0x7A7A7A7A, 0x85858585, 0x7B7B7B7B, 0x84848484, 0x7C7C7C7C, 0x83838383, 0x7D7D7D7D, 0x82828282, 0x7E7E7E7E, 0x81818181, 0x7F7F7F7F, 0x80808080, 0x80808080 },
{ 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x04040404, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x09090909, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x10101010, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x19191919, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x24242424, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x31313131, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x40404040, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x51515151, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x64646464, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x79797979, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0x90909090, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xA9A9A9A9, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xC4C4C4C4, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1, 0xE1E1E1E1 },
{ 0x00000000, 0x02020202, 0x04040404, 0x06060606, 0x08080808, 0x0A0A0A0A, 0x0C0C0C0C, 0x0E0E0E0E, 0x10101010, 0x12121212, 0x14141414, 0x16161616, 0x18181818, 0x1A1A1A1A, 0x1C1C1C1C, 0x1E1E1E1E, 0x20202020, 0x22222222, 0x24242424, 0x26262626, 0x28282828, 0x2A2A2A2A, 0x2C2C2C2C, 0x2E2E2E2E, 0x30303030, 0x32323232, 0x34343434, 0x36363636, 0x38383838, 0x3A3A3A3A, 0x3C3C3C3C, 0x3E3E3E3E, 0x40404040, 0x42424242, 0x44444444, 0x46464646, 0x48484848, 0x4A4A4A4A, 0x4C4C4C4C, 0x4E4E4E4E, 0x50505050, 0x52525252, 0x54545454, 0x56565656, 0x58585858, 0x5A5A5A5A, 0x5C5C5C5C, 0x5E5E5E5E, 0x60606060, 0x62626262, 0x64646464, 0x66666666, 0x68686868, 0x6A6A6A6A, 0x6C6C6C6C, 0x6E6E6E6E, 0x70707070, 0x72727272, 0x74747474, 0x76767676, 0x78787878, 0x7A7A7A7A, 0x7C7C7C7C, 0x7E7E7E7E, 0x80808080, 0x82828282, 0x84848484, 0x86868686, 0x88888888, 0x8A8A8A8A, 0x8C8C8C8C, 0x8E8E8E8E, 0x90909090, 0x92929292, 0x94949494, 0x96969696, 0x98989898, 0x9A9A9A9A, 0x9C9C9C9C, 0x9E9E9E9E, 0xA0A0A0A0, 0xA2A2A2A2, 0xA4A4A4A4, 0xA6A6A6A6, 0xA8A8A8A8, 0xAAAAAAAA, 0xACACACAC, 0xAEAEAEAE, 0xB0B0B0B0, 0xB2B2B2B2, 0xB4B4B4B4, 0xB6B6B6B6, 0xB8B8B8B8, 0xBABABABA, 0xBCBCBCBC, 0xBEBEBEBE, 0xC0C0C0C0, 0xC2C2C2C2, 0xC4C4C4C4, 0xC6C6C6C6, 0xC8C8C8C8, 0xCACACACA, 0xCCCCCCCC, 0xCECECECE, 0xD0D0D0D0, 0xD2D2D2D2, 0xD4D4D4D4, 0xD6D6D6D6, 0xD8D8D8D8, 0xDADADADA, 0xDCDCDCDC, 0xDEDEDEDE, 0xE0E0E0E0, 0xE2E2E2E2, 0xE4E4E4E4, 0xE6E6E6E6, 0xE8E8E8E8, 0xEAEAEAEA, 0xECECECEC, 0xEEEEEEEE, 0xF0F0F0F0, 0xF2F2F2F2, 0xF4F4F4F4, 0xF6F6F6F6, 0xF8F8F8F8, 0xFAFAFAFA, 0xFCFCFCFC, 0xFEFEFEFE, 0x00000000, 0x02020202, 0x04040404, 0x06060606, 0x08080808, 0x0A0A0A0A, 0x0C0C0C0C, 0x0E0E0E0E, 0x10101010, 0x12121212, 0x14141414, 0x16161616, 0x18181818, 0x1A1A1A1A, 0x1C1C1C1C, 0x1E1E1E1E, 0x20202020, 0x22222222, 0x24242424, 0x26262626, 0x28282828, 0x2A2A2A2A, 0x2C2C2C2C, 0x2E2E2E2E, 0x30303030, 0x32323232, 0x34343434, 0x36363636, 0x38383838, 0x3A3A3A3A, 0x3C3C3C3C, 0x3E3E3E3E, 0x40404040, 0x42424242, 0x44444444, 0x46464646, 0x48484848, 0x4A4A4A4A, 0x4C4C4C4C, 0x4E4E4E4E, 0x50505050, 0x52525252, 0x54545454, 0x56565656, 0x58585858, 0x5A5A5A5A, 0x5C5C5C5C, 0x5E5E5E5E, 0x60606060, 0x62626262, 0x64646464, 0x66666666, 0x68686868, 0x6A6A6A6A, 0x6C6C6C6C, 0x6E6E6E6E, 0x70707070, 0x72727272, 0x74747474, 0x76767676, 0x78787878, 0x7A7A7A7A, 0x7C7C7C7C, 0x7E7E7E7E, 0x80808080, 0x82828282, 0x84848484, 0x86868686, 0x88888888, 0x8A8A8A8A, 0x8C8C8C8C, 0x8E8E8E8E, 0x90909090, 0x92929292, 0x94949494, 0x96969696, 0x98989898, 0x9A9A9A9A, 0x9C9C9C9C, 0x9E9E9E9E, 0xA0A0A0A0, 0xA2A2A2A2, 0xA4A4A4A4, 0xA6A6A6A6, 0xA8A8A8A8, 0xAAAAAAAA, 0xACACACAC, 0xAEAEAEAE, 0xB0B0B0B0, 0xB2B2B2B2, 0xB4B4B4B4, 0xB6B6B6B6, 0xB8B8B8B8, 0xBABABABA, 0xBCBCBCBC, 0xBEBEBEBE, 0xC0C0C0C0, 0xC2C2C2C2, 0xC4C4C4C4, 0xC6C6C6C6, 0xC8C8C8C8, 0xCACACACA, 0xCCCCCCCC, 0xCECECECE, 0xD0D0D0D0, 0xD2D2D2D2, 0xD4D4D4D4, 0xD6D6D6D6, 0xD8D8D8D8, 0xDADADADA, 0xDCDCDCDC, 0xDEDEDEDE, 0xE0E0E0E0, 0xE2E2E2E2, 0xE4E4E4E4, 0xE6E6E6E6, 0xE8E8E8E8, 0xEAEAEAEA, 0xECECECEC, 0xEEEEEEEE, 0xF0F0F0F0, 0xF2F2F2F2, 0xF4F4F4F4, 0xF6F6F6F6, 0xF8F8F8F8, 0xFAFAFAFA, 0xFCFCFCFC, 0xFEFEFEFE },
{ 0xFFFFFFFF, 0xFDFDFDFD, 0xFBFBFBFB, 0xF9F9F9F9, 0xF7F7F7F7, 0xF5F5F5F5, 0xF3F3F3F3, 0xF1F1F1F1, 0xEFEFEFEF, 0xEDEDEDED, 0xEBEBEBEB, 0xE9E9E9E9, 0xE7E7E7E7, 0xE5E5E5E5, 0xE3E3E3E3, 0xE1E1E1E1, 0xDFDFDFDF, 0xDDDDDDDD, 0xDBDBDBDB, 0xD9D9D9D9, 0xD7D7D7D7, 0xD5D5D5D5, 0xD3D3D3D3, 0xD1D1D1D1, 0xCFCFCFCF, 0xCDCDCDCD, 0xCBCBCBCB, 0xC9C9C9C9, 0xC7C7C7C7, 0xC5C5C5C5, 0xC3C3C3C3, 0xC1C1C1C1, 0xBFBFBFBF, 0xBDBDBDBD, 0xBBBBBBBB, 0xB9B9B9B9, 0xB7B7B7B7, 0xB5B5B5B5, 0xB3B3B3B3, 0xB1B1B1B1, 0xAFAFAFAF, 0xADADADAD, 0xABABABAB, 0xA9A9A9A9, 0xA7A7A7A7, 0xA5A5A5A5, 0xA3A3A3A3, 0xA1A1A1A1, 0x9F9F9F9F, 0x9D9D9D9D, 0x9B9B9B9B, 0x99999999, 0x97979797, 0x95959595, 0x93939393, 0x91919191, 0x8F8F8F8F, 0x8D8D8D8D, 0x8B8B8B8B, 0x89898989, 0x87878787, 0x85858585, 0x83838383, 0x81818181, 0x7F7F7F7F, 0x7D7D7D7D, 0x7B7B7B7B, 0x79797979, 0x77777777, 0x75757575, 0x73737373, 0x71717171, 0x6F6F6F6F, 0x6D6D6D6D, 0x6B6B6B6B, 0x69696969, 0x67676767, 0x65656565, 0x63636363, 0x61616161, 0x5F5F5F5F, 0x5D5D5D5D, 0x5B5B5B5B, 0x59595959, 0x57575757, 0x55555555, 0x53535353, 0x51515151, 0x4F4F4F4F, 0x4D4D4D4D, 0x4B4B4B4B, 0x49494949, 0x47474747, 0x45454545, 0x43434343, 0x41414141, 0x3F3F3F3F, 0x3D3D3D3D, 0x3B3B3B3B, 0x39393939, 0x37373737, 0x35353535, 0x33333333, 0x31313131, 0x2F2F2F2F, 0x2D2D2D2D, 0x2B2B2B2B, 0x29292929, 0x27272727, 0x25252525, 0x23232323, 0x21212121, 0x1F1F1F1F, 0x1D1D1D1D, 0x1B1B1B1B, 0x19191919, 0x17171717, 0x15151515, 0x13131313, 0x11111111, 0x0F0F0F0F, 0x0D0D0D0D, 0x0B0B0B0B, 0x09090909, 0x07070707, 0x05050505, 0x03030303, 0x01010101, 0xFFFFFFFF, 0xFDFDFDFD, 0xFBFBFBFB, 0xF9F9F9F9, 0xF7F7F7F7, 0xF5F5F5F5, 0xF3F3F3F3, 0xF1F1F1F1, 0xEFEFEFEF, 0xEDEDEDED, 0xEBEBEBEB, 0xE9E9E9E9, 0xE7E7E7E7, 0xE5E5E5E5, 0xE3E3E3E3, 0xE1E1E1E1, 0xDFDFDFDF, 0xDDDDDDDD, 0xDBDBDBDB, 0xD9D9D9D9, 0xD7D7D7D7, 0xD5D5D5D5, 0xD3D3D3D3, 0xD1D1D1D1, 0xCFCFCFCF, 0xCDCDCDCD, 0xCBCBCBCB, 0xC9C9C9C9, 0xC7C7C7C7, 0xC5C5C5C5, 0xC3C3C3C3, 0xC1C1C1C1, 0xBFBFBFBF, 0xBDBDBDBD, 0xBBBBBBBB, 0xB9B9B9B9, 0xB7B7B7B7, 0xB5B5B5B5, 0xB3B3B3B3, 0xB1B1B1B1, 0xAFAFAFAF, 0xADADADAD, 0xABABABAB, 0xA9A9A9A9, 0xA7A7A7A7, 0xA5A5A5A5, 0xA3A3A3A3, 0xA1A1A1A1, 0x9F9F9F9F, 0x9D9D9D9D, 0x9B9B9B9B, 0x99999999, 0x97979797, 0x95959595, 0x93939393, 0x91919191, 0x8F8F8F8F, 0x8D8D8D8D, 0x8B8B8B8B, 0x89898989, 0x87878787, 0x85858585, 0x83838383, 0x81818181, 0x7F7F7F7F, 0x7D7D7D7D, 0x7B7B7B7B, 0x79797979, 0x77777777, 0x75757575, 0x73737373, 0x71717171, 0x6F6F6F6F, 0x6D6D6D6D, 0x6B6B6B6B, 0x69696969, 0x67676767, 0x65656565, 0x63636363, 0x61616161, 0x5F5F5F5F, 0x5D5D5D5D, 0x5B5B5B5B, 0x59595959, 0x57575757, 0x55555555, 0x53535353, 0x51515151, 0x4F4F4F4F, 0x4D4D4D4D, 0x4B4B4B4B, 0x49494949, 0x47474747, 0x45454545, 0x43434343, 0x41414141, 0x3F3F3F3F, 0x3D3D3D3D, 0x3B3B3B3B, 0x39393939, 0x37373737, 0x35353535, 0x33333333, 0x31313131, 0x2F2F2F2F, 0x2D2D2D2D, 0x2B2B2B2B, 0x29292929, 0x27272727, 0x25252525, 0x23232323, 0x21212121, 0x1F1F1F1F, 0x1D1D1D1D, 0x1B1B1B1B, 0x19191919, 0x17171717, 0x15151515, 0x13131313, 0x11111111, 0x0F0F0F0F, 0x0D0D0D0D, 0x0B0B0B0B, 0x09090909, 0x07070707, 0x05050505, 0x03030303, 0x01010101 },
{ 0xFFFFFFFF, 0xFDFDFDFD, 0xFBFBFBFB, 0xF9F9F9F9, 0xF7F7F7F7, 0xF5F5F5F5, 0xF3F3F3F3, 0xF1F1F1F1, 0xEFEFEFEF, 0xEDEDEDED, 0xEBEBEBEB, 0xE9E9E9E9, 0xE7E7E7E7, 0xE5E5E5E5, 0xE3E3E3E3, 0xE1E1E1E1, 0xDFDFDFDF, 0xDDDDDDDD, 0xDBDBDBDB, 0xD9D9D9D9, 0xD7D7D7D7, 0xD5D5D5D5, 0xD3D3D3D3, 0xD1D1D1D1, 0xCFCFCFCF, 0xCDCDCDCD, 0xCBCBCBCB, 0xC9C9C9C9, 0xC7C7C7C7, 0xC5C5C5C5, 0xC3C3C3C3, 0xC1C1C1C1, 0xBFBFBFBF, 0xBDBDBDBD, 0xBBBBBBBB, 0xB9B9B9B9, 0xB7B7B7B7, 0xB5B5B5B5, 0xB3B3B3B3, 0xB1B1B1B1, 0xAFAFAFAF, 0xADADADAD, 0xABABABAB, 0xA9A9A9A9, 0xA7A7A7A7, 0xA5A5A5A5, 0xA3A3A3A3, 0xA1A1A1A1, 0x9F9F9F9F, 0x9D9D9D9D, 0x9B9B9B9B, 0x99999999, 0x97979797, 0x95959595, 0x93939393, 0x91919191, 0x8F8F8F8F, 0x8D8D8D8D, 0x8B8B8B8B, 0x89898989, 0x87878787, 0x85858585, 0x83838383, 0x81818181, 0x7F7F7F7F, 0x7D7D7D7D, 0x7B7B7B7B, 0x79797979, 0x77777777, 0x75757575, 0x73737373, 0x71717171, 0x6F6F6F6F, 0x6D6D6D6D, 0x6B6B6B6B, 0x69696969, 0x67676767, 0x65656565, 0x63636363, 0x61616161, 0x5F5F5F5F, 0x5D5D5D5D, 0x5B5B5B5B, 0x59595959, 0x57575757, 0x55555555, 0x53535353, 0x51515151, 0x4F4F4F4F, 0x4D4D4D4D, 0x4B4B4B4B, 0x49494949, 0x47474747, 0x45454545, 0x43434343, 0x41414141, 0x3F3F3F3F, 0x3D3D3D3D, 0x3B3B3B3B, 0x39393939, 0x37373737, 0x35353535, 0x33333333, 0x31313131, 0x2F2F2F2F, 0x2D2D2D2D, 0x2B2B2B2B, 0x29292929, 0x27272727, 0x25252525, 0x23232323, 0x21212121, 0x1F1F1F1F, 0x1D1D1D1D, 0x1B1B1B1B, 0x19191919, 0x17171717, 0x15151515, 0x13131313, 0x11111111, 0x0F0F0F0F, 0x0D0D0D0D, 0x0B0B0B0B, 0x09090909, 0x07070707, 0x05050505, 0x03030303, 0x01010101, 0x00000000, 0x02020202, 0x04040404, 0x06060606, 0x08080808, 0x0A0A0A0A, 0x0C0C0C0C, 0x0E0E0E0E, 0x10101010, 0x12121212, 0x14141414, 0x16161616, 0x18181818, 0x1A1A1A1A, 0x1C1C1C1C, 0x1E1E1E1E, 0x20202020, 0x22222222, 0x24242424, 0x26262626, 0x28282828, 0x2A2A2A2A, 0x2C2C2C2C, 0x2E2E2E2E, 0x30303030, 0x32323232, 0x34343434, 0x36363636, 0x38383838, 0x3A3A3A3A, 0x3C3C3C3C, 0x3E3E3E3E, 0x40404040, 0x42424242, 0x44444444, 0x46464646, 0x48484848, 0x4A4A4A4A, 0x4C4C4C4C, 0x4E4E4E4E, 0x50505050, 0x52525252, 0x54545454, 0x56565656, 0x58585858, 0x5A5A5A5A, 0x5C5C5C5C, 0x5E5E5E5E, 0x60606060, 0x62626262, 0x64646464, 0x66666666, 0x68686868, 0x6A6A6A6A, 0x6C6C6C6C, 0x6E6E6E6E, 0x70707070, 0x72727272, 0x74747474, 0x76767676, 0x78787878, 0x7A7A7A7A, 0x7C7C7C7C, 0x7E7E7E7E, 0x80808080, 0x82828282, 0x84848484, 0x86868686, 0x88888888, 0x8A8A8A8A, 0x8C8C8C8C, 0x8E8E8E8E, 0x90909090, 0x92929292, 0x94949494, 0x96969696, 0x98989898, 0x9A9A9A9A, 0x9C9C9C9C, 0x9E9E9E9E, 0xA0A0A0A0, 0xA2A2A2A2, 0xA4A4A4A4, 0xA6A6A6A6, 0xA8A8A8A8, 0xAAAAAAAA, 0xACACACAC, 0xAEAEAEAE, 0xB0B0B0B0, 0xB2B2B2B2, 0xB4B4B4B4, 0xB6B6B6B6, 0xB8B8B8B8, 0xBABABABA, 0xBCBCBCBC, 0xBEBEBEBE, 0xC0C0C0C0, 0xC2C2C2C2, 0xC4C4C4C4, 0xC6C6C6C6, 0xC8C8C8C8, 0xCACACACA, 0xCCCCCCCC, 0xCECECECE, 0xD0D0D0D0, 0xD2D2D2D2, 0xD4D4D4D4, 0xD6D6D6D6, 0xD8D8D8D8, 0xDADADADA, 0xDCDCDCDC, 0xDEDEDEDE, 0xE0E0E0E0, 0xE2E2E2E2, 0xE4E4E4E4, 0xE6E6E6E6, 0xE8E8E8E8, 0xEAEAEAEA, 0xECECECEC, 0xEEEEEEEE, 0xF0F0F0F0, 0xF2F2F2F2, 0xF4F4F4F4, 0xF6F6F6F6, 0xF8F8F8F8, 0xFAFAFAFA, 0xFCFCFCFC, 0xFEFEFEFE },
{ 0x00000000, 0x02020202, 0x04040404, 0x06060606, 0x08080808, 0x0A0A0A0A, 0x0C0C0C0C, 0x0E0E0E0E, 0x10101010, 0x12121212, 0x14141414, 0x16161616, 0x18181818, 0x1A1A1A1A, 0x1C1C1C1C, 0x1E1E1E1E, 0x20202020, 0x22222222, 0x24242424, 0x26262626, 0x28282828, 0x2A2A2A2A, 0x2C2C2C2C, 0x2E2E2E2E, 0x30303030, 0x32323232, 0x34343434, 0x36363636, 0x38383838, 0x3A3A3A3A, 0x3C3C3C3C, 0x3E3E3E3E, 0x40404040, 0x42424242, 0x44444444, 0x46464646, 0x48484848, 0x4A4A4A4A, 0x4C4C4C4C, 0x4E4E4E4E, 0x50505050, 0x52525252, 0x54545454, 0x56565656, 0x58585858, 0x5A5A5A5A, 0x5C5C5C5C, 0x5E5E5E5E, 0x60606060, 0x62626262, 0x64646464, 0x66666666, 0x68686868, 0x6A6A6A6A, 0x6C6C6C6C, 0x6E6E6E6E, 0x70707070, 0x72727272, 0x74747474, 0x76767676, 0x78787878, 0x7A7A7A7A, 0x7C7C7C7C, 0x7E7E7E7E, 0x80808080, 0x82828282, 0x84848484, 0x86868686, 0x88888888, 0x8A8A8A8A, 0x8C8C8C8C, 0x8E8E8E8E, 0x90909090, 0x92929292, 0x94949494, 0x96969696, 0x98989898, 0x9A9A9A9A, 0x9C9C9C9C, 0x9E9E9E9E, 0xA0A0A0A0, 0xA2A2A2A2, 0xA4A4A4A4, 0xA6A6A6A6, 0xA8A8A8A8, 0xAAAAAAAA, 0xACACACAC, 0xAEAEAEAE, 0xB0B0B0B0, 0xB2B2B2B2, 0xB4B4B4B4, 0xB6B6B6B6, 0xB8B8B8B8, 0xBABABABA, 0xBCBCBCBC, 0xBEBEBEBE, 0xC0C0C0C0, 0xC2C2C2C2, 0xC4C4C4C4, 0xC6C6C6C6, 0xC8C8C8C8, 0xCACACACA, 0xCCCCCCCC, 0xCECECECE, 0xD0D0D0D0, 0xD2D2D2D2, 0xD4D4D4D4, 0xD6D6D6D6, 0xD8D8D8D8, 0xDADADADA, 0xDCDCDCDC, 0xDEDEDEDE, 0xE0E0E0E0, 0xE2E2E2E2, 0xE4E4E4E4, 0xE6E6E6E6, 0xE8E8E8E8, 0xEAEAEAEA, 0xECECECEC, 0xEEEEEEEE, 0xF0F0F0F0, 0xF2F2F2F2, 0xF4F4F4F4, 0xF6F6F6F6, 0xF8F8F8F8, 0xFAFAFAFA, 0xFCFCFCFC, 0xFEFEFEFE, 0xFFFFFFFF, 0xFDFDFDFD, 0xFBFBFBFB, 0xF9F9F9F9, 0xF7F7F7F7, 0xF5F5F5F5, 0xF3F3F3F3, 0xF1F1F1F1, 0xEFEFEFEF, 0xEDEDEDED, 0xEBEBEBEB, 0xE9E9E9E9, 0xE7E7E7E7, 0xE5E5E5E5, 0xE3E3E3E3, 0xE1E1E1E1, 0xDFDFDFDF, 0xDDDDDDDD, 0xDBDBDBDB, 0xD9D9D9D9, 0xD7D7D7D7, 0xD5D5D5D5, 0xD3D3D3D3, 0xD1D1D1D1, 0xCFCFCFCF, 0xCDCDCDCD, 0xCBCBCBCB, 0xC9C9C9C9, 0xC7C7C7C7, 0xC5C5C5C5, 0xC3C3C3C3, 0xC1C1C1C1, 0xBFBFBFBF, 0xBDBDBDBD, 0xBBBBBBBB, 0xB9B9B9B9, 0xB7B7B7B7, 0xB5B5B5B5, 0xB3B3B3B3, 0xB1B1B1B1, 0xAFAFAFAF, 0xADADADAD, 0xABABABAB, 0xA9A9A9A9, 0xA7A7A7A7, 0xA5A5A5A5, 0xA3A3A3A3, 0xA1A1A1A1, 0x9F9F9F9F, 0x9D9D9D9D, 0x9B9B9B9B, 0x99999999, 0x97979797, 0x95959595, 0x93939393, 0x91919191, 0x8F8F8F8F, 0x8D8D8D8D, 0x8B8B8B8B, 0x89898989, 0x87878787, 0x85858585, 0x83838383, 0x81818181, 0x7F7F7F7F, 0x7D7D7D7D, 0x7B7B7B7B, 0x79797979, 0x77777777, 0x75757575, 0x73737373, 0x71717171, 0x6F6F6F6F, 0x6D6D6D6D, 0x6B6B6B6B, 0x69696969, 0x67676767, 0x65656565, 0x63636363, 0x61616161, 0x5F5F5F5F, 0x5D5D5D5D, 0x5B5B5B5B, 0x59595959, 0x57575757, 0x55555555, 0x53535353, 0x51515151, 0x4F4F4F4F, 0x4D4D4D4D, 0x4B4B4B4B, 0x49494949, 0x47474747, 0x45454545, 0x43434343, 0x41414141, 0x3F3F3F3F, 0x3D3D3D3D, 0x3B3B3B3B, 0x39393939, 0x37373737, 0x35353535, 0x33333333, 0x31313131, 0x2F2F2F2F, 0x2D2D2D2D, 0x2B2B2B2B, 0x29292929, 0x27272727, 0x25252525, 0x23232323, 0x21212121, 0x1F1F1F1F, 0x1D1D1D1D, 0x1B1B1B1B, 0x19191919, 0x17171717, 0x15151515, 0x13131313, 0x11111111, 0x0F0F0F0F, 0x0D0D0D0D, 0x0B0B0B0B, 0x09090909, 0x07070707, 0x05050505, 0x03030303, 0x01010101 },
{ 0xFFFFFFFF, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xF6F6F6F6, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xE6E6E6E6, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xDBDBDBDB, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xCECECECE, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0xAEAEAEAE, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x9B9B9B9B, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x86868686, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x6F6F6F6F, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x56565656, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x3B3B3B3B, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E, 0x1E1E1E1E }
};

#pragma endregion

fb::cryptor::cryptor() : cryptor(0, (uint8_t*)"NexonInc.")
{}

fb::cryptor::cryptor(uint8_t types, uint8_t* key) : _key(NULL), _sequence(0), _type(types)
{
    this->_key = new uint8_t[0x09 * 4];
    for(int i = 0; i < 4; i++)
        memcpy(this->_key + 0x09 * i, key, 0x09);
}

fb::cryptor::cryptor(const cryptor& crt) : _sequence(0)
{
    this->operator=(crt);
}

fb::cryptor::~cryptor()
{
    delete[] this->_key;
}

void fb::cryptor::intercrypt(const uint8_t* core, uint8_t* dest, uint32_t size, const uint8_t* key, uint32_t ksize)
{
    uint8_t*                dest_offset = dest;
    const uint8_t*          src_offset  = core;
    uint32_t                num_loop    = size / 4;


    for(uint32_t i = 0; i < num_loop; i++)
    {
        unsigned int*       cvtint_dest = (unsigned int*)dest_offset;
        unsigned int*       cvtint_src  = (unsigned int*)src_offset;
        unsigned int*       cvtint_key  = (unsigned int*)key + (i % ksize);

        *cvtint_dest = *cvtint_src ^ *cvtint_key;

        src_offset  += 4;
        dest_offset += 4;
    }

    uint32_t                matched = size & 3;
    if(matched == 0)
        return;

    uint32_t                result     = matched - 1;
    uint32_t*               cvtint_key = (unsigned int*)key + (num_loop % ksize);
    uint32_t                cvtint_val = *cvtint_key;

    switch(matched)
    {
    case 1:
        *dest_offset = cvtint_val ^ *src_offset;
        break;

    case 2:
        result = cvtint_val ^ *(unsigned short *)src_offset;
        *(uint16_t*)dest_offset = result;
        break;

    case 3:
        *(uint16_t*)dest_offset = cvtint_val ^ *(unsigned short *)src_offset;
        result = src_offset[2];
        dest_offset[2] = result ^ (cvtint_val >> 16);
        break;
    }
}

uint32_t fb::cryptor::encrypt(fb::buffer& data, uint32_t offset, uint32_t size)
{
    const uint8_t*          data_c   = data.data() + offset;
    uint8_t*                buffer   = new uint8_t[size + 0x100];
    uint8_t                 key_size = 0x09;

    try
    {
        buffer[0] = data_c[0];
        buffer[1] = this->_sequence;

        if(size - 1 <= 0)
            throw;

        this->intercrypt(data_c + 1, buffer + 2, size - 1, this->_key, key_size);

        uint32_t            num_loop = (size - 2) / key_size + 1;
        if(num_loop > 0)
        {
            uint8_t*        offset = buffer + 2;
            for(uint32_t i = 0; i < num_loop; i++)
            {
                if(i != this->_sequence)
                    this->intercrypt(offset, offset, key_size, ((const uint8_t*)HEX_TABLE[this->_type]) + i * 4, 1);

                offset += key_size;
            }
        }
        this->intercrypt(buffer + 2, buffer + 2, size - 1, ((const uint8_t*)HEX_TABLE[this->_type]) + this->_sequence * 4, 1);
    }
    catch(...)
    {
        // DO NOTHING
    }

    uint32_t                    new_size = size + 1;

    buffer[new_size] = 0;
    data.erase(data.begin() + offset, data.begin() + offset + size);
    data.insert(data.begin() + offset, buffer, buffer + new_size);
    delete[] buffer;

    this->_sequence++;

    return new_size;
}

uint32_t fb::cryptor::encrypt(fb::buffer& data)
{
    return this->encrypt(data, 0, data.size());
}

uint32_t fb::cryptor::decrypt(fb::buffer& data, uint32_t offset, uint32_t size)
{
    const uint8_t*          data_c   = data.data() + offset;
    uint8_t*                buffer   = new uint8_t[size + 0x100];
    uint8_t                 key_size = 0x09;

    try
    {
        buffer[0] = data_c[0];
        uint8_t             sequence = data_c[1];

        if(size < 1)
            throw;

        this->intercrypt(data_c + 2, buffer + 1, size - 2, ((const uint8_t*)HEX_TABLE[this->_type]) + sequence * 4, 1);
        int num_subenc = (size - 3) / key_size + 1;
        if(num_subenc > 0)
        {
            uint8_t*        offset = buffer + 1;
            for(int i = 0; i < num_subenc; i++)
            {
                if(sequence != i)
                    this->intercrypt(offset, offset, key_size, ((const uint8_t*)HEX_TABLE[this->_type]) + i * 4, 1);

                offset += key_size;
            }
        }

        this->intercrypt(buffer + 1, buffer + 1, size - 2, this->_key, key_size);

    }
    catch(...)
    {
    }

    uint32_t                new_size = size - 1;

    buffer[new_size] = 0;
    data.erase(data.begin() + offset, data.begin() + offset + size);
    data.insert(data.begin() + offset, buffer, buffer + new_size);
    delete[] buffer;

    return new_size;
}

uint32_t fb::cryptor::decrypt(fb::buffer& data)
{
    return this->decrypt(data, 0, data.size());
}

uint32_t fb::cryptor::wrap(fb::buffer& data, uint32_t offset)
{
    uint16_t            size = (uint16_t)data.size() - offset;
    if(size == 0)
        return -1;

    const uint8_t       header[] = {0xAA, uint8_t(size >> 8 & 0xFF), uint8_t(size & 0xFF)};
    data.insert(data.begin() + offset, header, header + sizeof(header));

    return size + sizeof(header);
}

uint32_t fb::cryptor::wrap(fb::buffer& data)
{
    return this->wrap(data, 0);
}

uint32_t fb::cryptor::unwrap(fb::buffer& data, uint32_t offset)
{
    uint16_t            size = uint16_t(data.size() - offset);
    if(size < 3)
        return -1;

    data.erase(data.begin() + offset, data.begin() + offset + 3);
    return size - 3;
}

uint32_t fb::cryptor::unwrap(fb::buffer& data)
{
    return this->unwrap(data, 0);
}

uint8_t fb::cryptor::types() const
{
    return this->_type;
}

uint8_t* fb::cryptor::key() const
{
    return this->_key;
}

fb::cryptor& fb::cryptor::operator=(const cryptor& crt)
{
    for(int i = 0; i < 4; i++)
        memcpy(this->_key + 0x09 * i, (const void*)crt._key, 0x09);
    this->_type = crt._type;

    return *this;
}