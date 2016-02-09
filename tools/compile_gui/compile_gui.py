import json
import struct
import sys

BYTE_ORDER = '<'

constants = []

def declare_const(name, value):
    constants.append({
        "name": name,
        "value": value
    })
    globals()[name] = value

declare_const('SMALL_FONT', 1)
declare_const('MEDIUM_FONT', 2)
declare_const('LARGE_FONT', 3)
DEFAULT_FONT = MEDIUM_FONT

declare_const('BOX_FLAGS_BORDER', 1 << 0)

declare_const('BOX_FLAGS_HORZ_ALIGN', 3 << 1)
declare_const('BOX_FLAGS_HORZ_ALIGN_LEFT', 0 << 1)
declare_const('BOX_FLAGS_HORZ_ALIGN_RIGHT', 1 << 1)
declare_const('BOX_FLAGS_HORZ_ALIGN_CENTER', 2 << 1)

declare_const('BOX_FLAGS_VERT_ALIGN', 3 << 3)
declare_const('BOX_FLAGS_VERT_ALIGN_TOP', 0 << 3)
declare_const('BOX_FLAGS_VERT_ALIGN_BOTTOM', 1 << 3)
declare_const('BOX_FLAGS_VERT_ALIGN_CENTER', 2 << 3)

class Output:
    def __init__(self, single_indent="    "):
        self.single_indent = single_indent
        self.total_indent = ""
        self.num_errors = 0
        self.num_warnings = 0

    def indent(self):
        self.total_indent += self.single_indent

    def unindent(self):
        self.total_indent = self.total_indent[:len(self.total_indent) - len(self.single_indent)]

    def info(self, message):
        print(self.total_indent + message)

    def warning(self, message):
        print(self.total_indent + "WARNING: " + message)
        self.num_warnings += 1

    def error(self, message):
        print(self.total_indent + "ERROR: " + message)
        self.num_errors += 1

    def fatal_error(self, message):
        self.output_error(message)
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
            field.finish(objects)
            offset += field.size

        self.size = 2
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

        self.size = 4

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
        self.size = 2
        self.object_size = len(self.value) + 1

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.object_offset)

    def pack_object(self):
        packed_data = self.value.encode()
        packed_data.append(0)
        return packed_data

    def c_type(self):
        return "OBJ_OFFSET"

class UInt8(Field):
    def __init__(self, name, value):
        super().__init__(name)
        self.value = value

    def finish(self, objects):
        self.size = 1

    def pack(self):
        return struct.pack(BYTE_ORDER + 'B', self.value)

    def c_type(self):
        return "uint8_t"

class UInt16(Field):
    def __init__(self, name, value):
        super().__init__(name)
        self.value = value

    def finish(self, objects):
        self.size = 2

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.value)

    def c_type(self):
        return "uint16_t"
    
class Parser:
    def __init__(self, model_file_path, view_file_path):
        self.trace = Output()

        try:
            file = open(model_file_path)
        except Exception as e:
            self.trace.error("model file open error: %s" % e)
            return

        try:
            model = json.loads(file.read())
        except Exception as e:
            self.trace.error("model file format error: %s" % e)
            return

        try:
            file = open(view_file_path)
        except Exception as e:
            self.trace.error("view file open error: %s" % e)
            return

        try:
            view = json.loads(file.read())
        except Exception as e:
            self.trace.error("view file format error: %s" % e)
            return

        self.model = model
        self.view = view

        self.parse_document()

    def num_errors(self):
        return self.trace.num_errors

    def num_warnings(self):
        return self.trace.num_warnings

    def parse_document(self):
        self.document = Struct("document", "Document")

        styles = List("styles")
        for style in self.view["styles"]:
            styles.addItem(self.parse_style(style))
        self.document.addField(styles)

        pages = List("pages")
        for page in self.view["pages"]:
            pages.addItem(self.parse_page(page))
        self.document.addField(pages)

    def parse_style(self, style):
        result = Struct('style', 'Style')

        if not "name" in style:
            self.trace.error("unnamed style found")
            return result

        self.trace.info("Style '%s'" % style["name"])

        self.trace.indent()

        if "font" in style:
            font_str = style['font'].lower()
            if font_str == 'small':
                fotn = SMALL_FONT
            elif font_str == 'medium':
                font = MEDIUM_FONT
            elif font_str == 'large':
                font = LARGE_FONT
            else:
                self.trace.error("unknown font '%s'" % style['font'])
                font = DEFAULT_FONT
        else:
            font = DEFAULT_FONT
        result.addField(UInt8('font', font))

        self.trace.unindent()

        return result

    def parse_page(self, page):
        if "name" in page:
            self.trace.info("Page '%s'" % page["name"])
        else:
            self.trace.info("Page '<noname>'")

        self.trace.indent()

        if not "name" in page:
            self.trace.warning("unnamed page found")

        result = Struct('page', 'Page')

        if "width" in page:
            result.addField(UInt16('width', page['width']))
        else:
            self.trace.error("width is missing")

        if "height" in page:
            result.addField(UInt16('height', page['height']))
        else:
            self.trace.error("height is missing")

        if not "widgets" in page:
            self.trace.error("widgets is missing")
        else:
            widgets = List("widgets")
            for index, widget in enumerate(page["widgets"]):
                widgets.addItem(self.parse_widget(index, widget))
            result.addField(widgets)

        self.trace.unindent()

        return result

    def parse_widget(self, index, widget):
        self.trace.info("Widget %d" % index)
        self.trace.indent()

        result = Struct('widget', "Widget")

        if "x" in widget:
            result.addField(UInt16('x', widget['x']))
        else:
            self.trace.error("x is missing")

        if "y" in widget:
            result.addField(UInt16('y', widget['y']))
        else:
            self.trace.error("y is missing")

        if "w" in widget:
            result.addField(UInt16('w', widget['w']))
        else:
            self.trace.error("w is missing")

        if "h" in widget:
            result.addField(UInt16('h', widget['h']))
        else:
            self.trace.error("h is missing")

        list_widgets = List("widgets")
        if "widgets" in widget:
            for index, list_widget in enumerate(widget["widgets"]):
                list_widgets.addItem(self.parse_widget(index, list_widget))
        result.addField(list_widgets)

        self.trace.unindent()

        return result

def finish(document):
    objects = []
    document.finish(objects)
    
    object_offset = 0
    for object in objects:
        object.object_offset = object_offset
        object_offset += object.object_size

    return objects

def generate_source_code(objects, tab):
    result = ""

    #generate constants
    for constant in constants:
        result += "#define " + constant["name"] + " " + str(constant["value"]) + "\n"
    result += "\n"

    #
    result += "typedef uint16_t OBJ_OFFSET;\n\n"

    result += "struct List {\n"
    result += tab + "uint16_t count;\n"
    result += tab + "OBJ_OFFSET first;\n"
    result += "};\n\n"

    # generate structures
    structs = {}

    for object in objects:
        if isinstance(object, Struct):
            structs[object.struct_name] = object

    for struct in structs.values():
        result += "struct " + struct.struct_name + " {\n"
        for field in struct.fields:
            result += tab + field.c_type() + " " + field.name + ";\n"
        result += "};\n\n"

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
    

parser = Parser("./model.json", "./view.json")
print("\n%d errors, %d warnings" % (parser.num_errors(), parser.num_warnings()))

if parser.num_errors() == 0:
    objects = finish(parser.document)

    c_structs = generate_source_code(objects, "    ")
    print(c_structs, end="")

    packed_data = pack(objects)
    c_buffer = to_c_buffer(packed_data, "    ", 16)
    print("uint8_t document[] = " + c_buffer + ";")
