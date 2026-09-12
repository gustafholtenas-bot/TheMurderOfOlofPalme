#include "../../Plugins/TMOPEngine/Source/TMOPEngine/Private/People/TMOPPersonNameFormatting.h"
#include <clocale>
#include <cwctype>
#include <iostream>

bool Letter(wchar_t Ch) { return std::iswalpha(Ch); }
wchar_t Upper(wchar_t Ch) { return static_cast<wchar_t>(std::towupper(Ch)); }
int main()
{
    if (!std::setlocale(LC_CTYPE, "C.UTF-8")) return 2;
    struct Case { const wchar_t* Full; const wchar_t* First; const wchar_t* Last; const wchar_t* Expected; };
    const Case Cases[] = {
        {L"Anders Björkman",L"Anders",L"Björkman",L"Anders B."},
        {L"Olof Palme",L"",L"",L"Olof P."},
        {L"Lisbeth Palme",L"",L"",L"Lisbeth P."},
        {L"Jan-Åke Svensson",L"",L"",L"Jan-Åke S."},
        {L"Johan Gustaf Gunnarsson",L"Johan Gustaf",L"Gunnarsson",L"Johan Gustaf G."},
        {L"Johan Gustaf Gunnarsson",L"",L"",L"Johan G. G."},
        {L"Elisabeth Lönn Sunde",L"",L"",L"Elisabeth L. S."},
        {L"Elisabeth Lönn Sunde",L"Elisabeth",L"Lönn Sunde",L"Elisabeth L."},
        {L"Anna af Ugglas",L"Anna",L"af Ugglas",L"Anna A."},
        {L"Anna af Ugglas",L"",L"",L"Anna A. U."},
        {L"Åke Åbrink",L"",L"",L"Åke Å."},
        {L"G. Andersson",L"",L"",L"G. A."},
        {L"Anders B.",L"",L"",L"Anders B."},
        {L"Hans Johansson (taxichaufför)",L"",L"",L"Hans J. (taxichaufför)"},
        {L"Åke Röst, uppgiftsmottagare",L"",L"",L"Åke R., uppgiftsmottagare"},
        {L"  Anders\tBjörkman  ",L"",L"",L"Anders B."},
        {L"Sven och Berith Larsson",L"",L"",L"Sven och Berith L."},
        {L"Sven Larsson & Berith Larsson",L"",L"",L"Sven L. & Berith L."},
        {L"Gärningsmannen",L"",L"",L"Gärningsmannen"},
        {L"Okänd man vid Grand",L"",L"",L"Okänd man vid Grand"},
        {L"Kerstin",L"",L"",L"Kerstin"},
        {L"Bondestam",L"",L"Bondestam",L"B."},
        {L"Anders Björkman",L"",L"Björkman",L"Anders B."},
        {L"",L"Anna",L"Andersson",L"Anna A."},
        {L"",L"",L"",L""},
    };
    for (const auto& C : Cases)
    {
        const auto Got = TMOPPersonNames::Format(std::wstring(C.Full),std::wstring(C.First),
            std::wstring(C.Last),Letter,Upper);
        if (Got != C.Expected) { std::wcerr << L"FAIL: " << C.Full << L" -> " << Got << L'\n'; return 1; }
        // Formatting a public name twice must not expose or further shorten the first name.
        if (TMOPPersonNames::Fallback(Got,Letter,Upper) != Got && !std::wstring(C.First).empty())
        {
            // Explicit multi-part given names must be formatted through their structured profile.
            if (std::wstring(C.First).find(L' ') == std::wstring::npos) return 1;
        }
    }
    std::cout << sizeof(Cases)/sizeof(Case) << " person-name cases passed\n";
}
