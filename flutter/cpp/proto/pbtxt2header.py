import os
import sys

HEADER_FILE_TEMPLATE = """
#ifndef {HEADER_NAME}
#define {HEADER_NAME}

#include <string>

const std::string {VARIABLE_NAME} = R"SETTINGS(

{VARIABLE_VALUE}

)SETTINGS";

#endif
"""


def main(in_fpath, out_fpath, template):
    basename = os.path.basename(in_fpath).replace(".", "_")
    header_name = basename.upper() + "_H"
    variable_name = basename

    template = template.replace("{HEADER_NAME}", header_name)
    template = template.replace("{VARIABLE_NAME}", variable_name)

    with open(in_fpath, 'r') as in_file:
        content = in_file.read()
        template = template.replace("{VARIABLE_VALUE}", content)

    with open(out_fpath, 'w') as out_file:
        out_file.write(template)


if __name__ == "__main__":
    """ Usage: python pbtxt2header.py <input_file> <output_file>"""
    in_filepath = sys.argv[1]
    out_filepath = sys.argv[2]
    main(in_filepath, out_filepath, HEADER_FILE_TEMPLATE)
