#!/bin/python3
import os
import sys
import json

def main():
    '''Converts the Aseprite export JSON to an RML sprite sheet'''
    if len(sys.argv) != 3:
        print(f"Usage: ./{sys.argv[0]} (input json) (output rcss)")
        return

    # Parse variables
    input_path = sys.argv[1]
    output_path = sys.argv[2]

    if not input_path.endswith(".json") or not output_path.endswith(".rcss"):
        # Guarantees json is input and css is output
        return

    # Open the JSON
    with open(input_path, "r") as json_handle, open(output_path, "w") as css_handle:
        json_data = json.loads(json_handle.read())
        json_meta = json_data["meta"]
        image_url = json_meta["image"]
        slices = json_meta["slices"]

        sheet_name = os.path.splitext(os.path.basename(image_url))[0]

        css_handle.write(f"@spritesheet {sheet_name}\n")
        css_handle.write("{\n")
        css_handle.write(f"\tsrc: {image_url};\n")
        css_handle.write("\tresolution: 1x;\n")

        for slice in slices:
            slice_name = slice["name"]
            bounds = slice["keys"][0]["bounds"]

            css_handle.write(f"\t{slice_name}: {bounds["x"]}px {bounds["y"]}px {bounds["w"]}px {bounds["h"]}px;\n")

        css_handle.write("}")

if __name__ == "__main__":
    main()
