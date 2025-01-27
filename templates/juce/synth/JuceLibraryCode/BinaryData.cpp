/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#include <cstring>

namespace BinaryData
{

//================== subtractive.k ==================
static const unsigned char temp_binary_data_0[] =
"\r\n"
"#include <klang.h>\r\n"
"using namespace klang::optimised;\r\n"
"\r\n"
"struct Subtractive : Stereo::Synth {\r\n"
"\r\n"
"\tstruct SubtractiveNote : public Note {\r\n"
"\t\tSquare osc;\r\n"
"\t\tADSR adsr;\r\n"
"\t\t\r\n"
"\t\tEnvelope env;\r\n"
"\t\tLPF filter;\r\n"
"\t\t\r\n"
"\t\tevent on(Pitch pitch, Amplitude velocity) {\r\n"
"\t\t\tparam f = pitch -> Frequency;\r\n"
"\t\t\tosc(f, 0);\r\n"
"\t\t\t\r\n"
"\t\t\tadsr(0,0,1,0.25);\r\n"
"\t\t\tenv = { { 0, f * 2 }, { 0.25, f * 10 }, { 2, f * 5 } };\r\n"
"\t\t\t\r\n"
"\t\t\tfilter.reset();\r\n"
"\t\t}\r\n"
"\r\n"
"\t\tevent off(Amplitude velocity) {\r\n"
"\t\t\tadsr.release();\r\n"
"\t\t}\r\n"
"\r\n"
"\t\tvoid process() {\r\n"
"\t\t\tosc >> filter(env++, 10) >> out;\r\n"
"\t\t\t\r\n"
"\t\t\tout *= adsr;\r\n"
"\t\t\tif (adsr.finished())\r\n"
"\t\t\t\tstop();\r\n"
"\t\t}\r\n"
"\t};\r\n"
"\r\n"
"\tSubtractive() {\r\n"
"\t\tnotes.add<SubtractiveNote>(32);\r\n"
"\t}\r\n"
"};";

const char* subtractive_k = (const char*) temp_binary_data_0;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0xfda58030:  numBytes = 674; return subtractive_k;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "subtractive_k"
};

const char* originalFilenames[] =
{
    "subtractive.k"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
        if (strcmp (namedResourceList[i], resourceNameUTF8) == 0)
            return originalFilenames[i];

    return nullptr;
}

}
