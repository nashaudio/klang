/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#include <cstring>

namespace BinaryData
{

//================== pingpong.k ==================
static const unsigned char temp_binary_data_0[] =
"\r\n"
"#include <klang.h>\r\n"
"using namespace klang::optimised;\r\n"
"\r\n"
"struct PingPong : public Stereo::Effect {\r\n"
"\tStereo::Delay<192000> delay;\r\n"
"\r\n"
"\t// Initialise plugin (called once at startup)\r\n"
"\tPingPong() {\r\n"
"\t\tcontrols = {\r\n"
"\t\t \t{ \"Left\",\r\n"
"\t\t\t\tDial(\"Delay\", 0.0, 1.0, 0.5 ),\r\n"
"\t\t\t\tDial(\"Feedback\", 0.0, 1.0, 0.5 )\r\n"
"\t\t\t},\r\n"
"\t\t\t{ \"Right\",\r\n"
"\t\t\t\tDial(\"Delay\", 0.0, 1.0, 0.5 ),\r\n"
"\t\t\t\tDial(\"Feedback\", 0.0, 1.0, 0.5 ),\r\n"
"\t\t\t}\r\n"
"\t\t};\r\n"
"\t}\r\n"
"\r\n"
"\t// Apply processing (called once per sample)\r\n"
"\tvoid process() {\r\n"
"\t\tstereo::signal time = { controls[0].smooth(), controls[2].smooth() };\r\n"
"\t\tstereo::signal gain = { controls[1], controls[3] };\r\n"
"\t\t\r\n"
"\t\tstereo::signal feedback = (out >> delay)(time * fs) * gain;\r\n"
"\t\t\r\n"
"\t\tin.l + feedback.r >> out.l;\r\n"
"\t\tin.r + feedback.l >> out.r;\r\n"
"\t}\r\n"
"};";

const char* pingpong_k = (const char*) temp_binary_data_0;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x6104a9d6:  numBytes = 751; return pingpong_k;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "pingpong_k"
};

const char* originalFilenames[] =
{
    "pingpong.k"
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
