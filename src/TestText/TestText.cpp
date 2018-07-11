#include <sdw.h>

int UMain(int argc, UChar* argv[])
{
	if (argc != 4)
	{
		return 1;
	}
	FILE* fp = UFopen(argv[1], USTR("rb"), false);
	if (fp == nullptr)
	{
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	u32 uTxtSize = ftell(fp);
	if (uTxtSize % 2 != 0)
	{
		fclose(fp);
		return 1;
	}
	uTxtSize /= 2;
	fseek(fp, 0, SEEK_SET);
	Char16_t* pTemp = new Char16_t[uTxtSize + 1];
	fread(pTemp, 2, uTxtSize, fp);
	fclose(fp);
	if (pTemp[0] != 0xFEFF)
	{
		delete[] pTemp;
		return 1;
	}
	pTemp[uTxtSize] = 0;
	wstring sTxt = U16ToW(pTemp + 1);
	delete[] pTemp;
	wstring sCharset;
	for (u32 i = 0; i < static_cast<u32>(sTxt.size()); i++)
	{
		u16 uUnicode = sTxt[i];
		if (uUnicode >= 0x20)
		{
			sCharset.append(1, uUnicode);
		}
	}
	fp = UFopen(argv[2], USTR("rb"), false);
	if (fp == nullptr)
	{
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	uTxtSize = ftell(fp);
	if (uTxtSize % 2 != 0)
	{
		fclose(fp);
		return 1;
	}
	uTxtSize /= 2;
	fseek(fp, 0, SEEK_SET);
	pTemp = new Char16_t[uTxtSize + 1];
	fread(pTemp, 2, uTxtSize, fp);
	fclose(fp);
	if (pTemp[0] != 0xFEFF)
	{
		delete[] pTemp;
		return 1;
	}
	pTemp[uTxtSize] = 0;
	sTxt = U16ToW(pTemp + 1);
	delete[] pTemp;
	n32 nCount = 0;
	map<n32, wstring> mTxt;
	wstring::size_type uPos0 = 0;
	while ((uPos0 = sTxt.find(L"No.", uPos0)) != wstring::npos)
	{
		uPos0 += wcslen(L"No.");
		n32 nNo = SToN32(sTxt.c_str() + uPos0);
		uPos0 = sTxt.find(L"\r\n--------------------------------------\r\n", uPos0);
		if (uPos0 == wstring::npos)
		{
			break;
		}
		uPos0 += wcslen(L"\r\n--------------------------------------\r\n");
		wstring::size_type uPos1 = sTxt.find(L"\r\n======================================\r\n", uPos0);
		if (uPos1 == wstring::npos)
		{
			break;
		}
		wstring sStmt = sTxt.substr(uPos0, uPos1 - uPos0);
		uPos0 = uPos1 + wcslen(L"\r\n======================================\r\n");
		static wregex r0(L"<.*?>", regex_constants::ECMAScript);
		sStmt = regex_replace(sStmt, r0, L"");
		static wregex r1(L"\\{\\d+\\}", regex_constants::ECMAScript);
		sStmt = regex_replace(sStmt, r1, L"");
		sStmt = Replace(sStmt, L"\r", L"");
		sStmt = Replace(sStmt, L"\n", L"");
		if (!sStmt.empty() && sStmt.find_first_not_of(sCharset) == wstring::npos)
		{
			mTxt.insert(make_pair(nNo, sStmt));
		}
		nCount++;
	}
	if (!mTxt.empty())
	{
		fp = UFopen(argv[3], USTR("ab"), false);
		if (fp == nullptr)
		{
			return 1;
		}
		fseek(fp, 0, SEEK_END);
		u32 uFileSize = ftell(fp);
		if (uFileSize == 0)
		{
			fwrite("\xFF\xFE", 2, 1, fp);
		}
		else
		{
			fu16printf(fp, L"\r\n\r\n");
		}
		fu16printf(fp, L"%d%% %ls\r\n", mTxt.size() * 100 / nCount, UToW(argv[2]).c_str());
		for (map<n32, wstring>::iterator it = mTxt.begin(); it != mTxt.end(); ++it)
		{
			fu16printf(fp, L"\tNo.%d,\t%ls\r\n", it->first, it->second.c_str());
		}
		fclose(fp);
	}
	return 0;
}
