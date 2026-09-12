#pragma once
#include <string>
#include <vector>

// Character classification comes from FChar in Unreal, and the Unicode locale in standalone tests.
namespace TMOPPersonNames
{
template<class C> bool Space(C Ch)
{ return Ch == C(' ') || Ch == C('\t') || Ch == C('\n') || Ch == C('\r') || Ch == C(0xa0); }

template<class C> std::basic_string<C> Trim(std::basic_string<C> S)
{
    const auto First = S.find_first_not_of(std::basic_string<C>{C(' '), C('\t'), C('\n'), C('\r'), C(0xa0)});
    if (First == S.npos) return {};
    const auto Last = S.find_last_not_of(std::basic_string<C>{C(' '), C('\t'), C('\n'), C('\r'), C(0xa0)});
    return S.substr(First, Last - First + 1);
}

template<class C, class Alpha, class Upper>
std::basic_string<C> Initial(const std::basic_string<C>& S, Alpha IsAlpha, Upper ToUpper)
{
    for (C Ch : S) if (IsAlpha(Ch)) return {ToUpper(Ch), C('.')};
    return {};
}

template<class C, class Alpha, class Upper>
std::basic_string<C> Fallback(const std::basic_string<C>& Input, Alpha IsAlpha, Upper ToUpper)
{
    using String = std::basic_string<C>;
    const String S = Trim(Input);
    // Named pairs keep each given name: "Sven och Berith Larsson" -> "Sven och Berith L.".
    for (const char* Separator : {" & ", " och "})
    {
        String Join;
        for (const char* P = Separator; *P; ++P) Join.push_back(C(*P));
        const auto Split = S.find(Join);
        if (Split != String::npos)
            return Fallback(S.substr(0, Split), IsAlpha, ToUpper) + Join +
                Fallback(S.substr(Split + Join.size()), IsAlpha, ToUpper);
    }
    std::vector<String> NameParts;
    std::size_t At = 0, EndOfName = 0;
    while (At < S.size())
    {
        while (At < S.size() && Space(S[At])) ++At;
        const auto Start = At;
        while (At < S.size() && !Space(S[At]) && S[At] != C('(') && S[At] != C(',') &&
            S[At] != C(':') && S[At] != C(';')) ++At;
        const String Word = S.substr(Start, At - Start);
        if (Word.empty()) break;
        bool Valid = IsAlpha(Word[0]);
        for (C Ch : Word)
            Valid = Valid && (IsAlpha(Ch) || Ch == C('-') || Ch == C('.') || Ch == C('\'') || Ch == C(0x2019));
        bool Particle = false;
        for (const char* P : {"af", "von", "van", "de", "del", "der", "den", "di", "da", "du", "la", "le"})
        {
            String Candidate;
            for (; *P; ++P) Candidate.push_back(C(*P));
            if (Word == Candidate) Particle = true;
        }
        if (!Valid || (Word[0] != ToUpper(Word[0]) && !Particle)) break;
        NameParts.push_back(Word);
        EndOfName = At;
    }
    // A single given name / alias / role has no identifiable surname to shorten.
    if (NameParts.size() < 2) return S;
    String Out = NameParts.front();
    // Without separate fields, shorten every remaining name part; compound surnames stay private.
    for (std::size_t I = 1; I < NameParts.size(); ++I)
        Out += String(1, C(' ')) + Initial(NameParts[I], IsAlpha, ToUpper);
    return Out + S.substr(EndOfName);
}

template<class C, class Alpha, class Upper>
std::basic_string<C> Format(const std::basic_string<C>& Full, const std::basic_string<C>& First,
    const std::basic_string<C>& Last, Alpha IsAlpha, Upper ToUpper)
{
    using String = std::basic_string<C>;
    const String Surname = Trim(Last), Given = Trim(First), Name = Trim(Full);
    if (Surname.empty()) return Name.empty() ? Given : Fallback(Name, IsAlpha, ToUpper);
    const String Abbreviation = Initial(Surname, IsAlpha, ToUpper);
    if (!Given.empty()) return Given + String(1, C(' ')) + Abbreviation;
    String UpperName = Name, UpperLast = Surname;
    for (auto& Ch : UpperName) Ch = ToUpper(Ch);
    for (auto& Ch : UpperLast) Ch = ToUpper(Ch);
    const auto Pos = UpperName.find(UpperLast);
    const String InferredGiven = Pos != String::npos ? Trim(Name.substr(0, Pos)) : String{};
    return InferredGiven.empty() ? Abbreviation : InferredGiven + String(1, C(' ')) + Abbreviation;
}
}
