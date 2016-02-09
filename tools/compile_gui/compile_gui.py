import json
import struct
import sys

BYTE_ORDER = '<'

SINGLE_INDENT = "    "
indent = ""
num_errors = 0
num_warnings = 0

def output_indent():
    global indent
    indent += SINGLE_INDENT

def output_unindent():
    global indent
    indent = indent[:len(indent) - len(SINGLE_INDENT)]

def output_info(message):
    print(indent + message)

def output_warning(message):
    print(indent + "WARNING: " + message)
    global num_warnings
    num_warnings += 1

def output_error(message):
    print(indent + "ERROR: " + message)
    global num_errors
    num_errors += 1

def output_fatal_error(message):
    output_error(message)
    sys.exit(1)

class Field:
    def __init__(self, name):
        self.name = name

class Struct(Field):
    def __init__(self, name, struct_name):
        super().__init__(name)
        self.fields = []
        self.struct_name = struct_name

    def addField(self, field):
        self.fields.append(field)

    def finish(self, objects):
        objects.append(self)

        offset = 0
        for field in self.fields:
            field.offset = offset
            field.finish(objects);
            offset += field.size

        self.size = 2;
        self.object_size = offset

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.object_offset)

    def pack_object(self):
        packed_data = []
        for field in self.fields:
            packed_data += field.pack()
        return packed_data

    def c_type(self):
        return "OBJ_OFFSET"

class List(Field):
    def __init__(self, name):
        super().__init__(name)
        self.items = []

    def addItem(self, item):
        self.items.append(item)

    def finish(self, objects):
        for item in self.items:
            item.finish(objects)

        self.size = 4;

    def pack(self):
        return struct.pack(BYTE_ORDER + 'HH', len(self.items), self.items[0].object_offset if len(self.items) > 0 else 0)

    def c_type(self):
        return "List"

class String(Field):
    def __init__(self, name, value):
        super().__init__(name)
        self.value = value

    def finish(self, objects):
        objects.append(self)
        self.size = 2;
        self.object_size = len(self.value) + 1

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.object_offset)

    def pack_object(self):
        packed_data = self.value.encode()
        packed_data.append(0)
        return packed_data

    def c_type(self):
        return "OBJ_OFFSET"


class UInt16(Field):
    def __init__(self, name, value):
        super().__init__(name)
        self.value = value

    def finish(self, objects):
        self.size = 2;

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.value)

    def c_type(self):
        return "uint16_t"

def parse_widget(index, widget):
    output_info("Widget %d" % index)
    output_indent()

    result = Struct('widget', "Widget")

    if "x" in widget:
        result.addField(UInt16('x', widget['x']))
    else:
        output_error("x is missing")

    if "y" in widget:
        result.addField(UInt16('y', widget['y']))
    else:
        output_error("y is missing")

    if "w" in widget:
        result.addField(UInt16('w', widget['w']))
    else:
        output_error("w is missing")

    if "h" in widget:
        result.addField(UInt16('h', widget['h']))
    else:
        output_error("h is missing")

    list_widgets = List("widgets")
    if "widgets" in widget:
        for index, list_widget in enumerate(widget["widgets"]):
            list_widgets.addItem(parse_widget(index, list_widget))
    result.addField(list_widgets)

    output_unindent()

    return result

def parse_page(page):
    if "name" in page:
        output_info("Page '%s'" % page["name"])
    else:
        output_info("Page '<noname>'")

    output_indent()

    if not "name" in page:
        output_warning("unnamed page found")

    result = Struct('page', 'Page')

    if "width" in page:
        result.addField(UInt16('width', page['width']))
    else:
        output_error("width is missing")

    if "height" in page:
        result.addField(UInt16('height', page['height']))
    else:
        output_error("height is missing")

    if not "widgets" in page:
        output_error("widgets is missing")
    else:
        widgets = List("widgets")
        for index, widget in enumerate(page["widgets"]):
            widgets.addItem(parse_widget(index, widget))
        result.addField(widgets)

    output_unindent()

    return result

def parse_document(model, view):
    document = Struct("document", "Document")

    pages = List("pages")
    for page in view["pages"]:
        pages.addItem(parse_page(page))

    document.addField(pages)

    return document

def parse(model_file_path, view_file_path):
    try:
        file = open(model_file_path)
    except Exception as e:
        output_fatal_error("model file open error: %s" % e)

    try:
        model = json.loads(file.read())
    except Exception as e:
        output_fatal_error("model file format error: %s" % e)

    try:
        file = open(view_file_path)
    except Exception as e:
        output_fatal_error("view file open error: %s" % e)

    try:
        view = json.loads(file.read())
    except Exception as e:
        output_fatal_error("view file format error: %s" % e)

    return parse_document(model, view)

def finish(document):
    objects = []
    document.finish(objects)
    
    object_offset = 0
    for object in objects:
        object.object_offset = object_offset
        object_offset += object.object_size

    return objects

def to_c_structs(objects, tab):
    structs = {}

    for object in objects:
        if isinstance(object, Struct):
            structs[object.struct_name] = object

    result = "typedef uint16_t OBJ_OFFSET;\n\n";

    result += "struct List {\n";
    result += tab + "uint16_t count;\n"
    result += tab + "OBJ_OFFSET first;\n"
    result += "};\n\n";

    for struct in structs.values():
        result += "struct " + struct.struct_name + " {\n";
        for field in struct.fields:
            result += tab + field.c_type() + " " + field.name + ";\n"
        result += "};\n\n";

    return result

def pack(objects):
    packed_data = []
    for object in objects:
        packed_data += object.pack_object()

    return packed_data

def to_c_buffer(self, tab, num_columns):
    c_buffer = "{"
    column_index = 0
    for byte in packed_data:
        if (column_index % num_columns) == 0:
            if column_index > 0:
                c_buffer += ','
            c_buffer += '\n' + tab
        else:
            c_buffer += ', '
        c_buffer += "0x%0.2X" % byte
        column_index += 1
    c_buffer += "\n}"
    return c_buffer
    

document = parse("./model.json", "./view.json")
print("\n%d errors, %d warnings" % (num_errors, num_warnings))

if num_errors == 0:
    objects = finish(document)

    c_structs = to_c_structs(objects, "    ")
    print(c_structs, end="")

    packed_data = pack(objects)
    c_buffer = to_c_buffer(packed_data, "    ", 16)
    print("uint8_t document[] = " + c_buffer + ";")
