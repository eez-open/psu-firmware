import json
import struct
import sys

BYTE_ORDER = '<'

#-------------------------------------------------------------------------------

constants = []

def declare_const(name, value):
    constants.append({
        'name': name,
        'value': value
    })
    globals()[name] = value

#-------------------------------------------------------------------------------

declare_const('SMALL_FONT', 1)
declare_const('MEDIUM_FONT', 2)
declare_const('LARGE_FONT', 3)
DEFAULT_FONT = MEDIUM_FONT

#-------------------------------------------------------------------------------

declare_const('STYLE_FLAGS_BORDER', 1 << 0)

declare_const('STYLE_FLAGS_HORZ_ALIGN', 3 << 1)
declare_const('STYLE_FLAGS_HORZ_ALIGN_LEFT', 0 << 1)
declare_const('STYLE_FLAGS_HORZ_ALIGN_RIGHT', 1 << 1)
declare_const('STYLE_FLAGS_HORZ_ALIGN_CENTER', 2 << 1)

declare_const('STYLE_FLAGS_VERT_ALIGN', 3 << 3)
declare_const('STYLE_FLAGS_VERT_ALIGN_TOP', 0 << 3)
declare_const('STYLE_FLAGS_VERT_ALIGN_BOTTOM', 1 << 3)
declare_const('STYLE_FLAGS_VERT_ALIGN_CENTER', 2 << 3)

#-------------------------------------------------------------------------------

declare_const('WIDGET_TYPE_NONE', 0)
declare_const('WIDGET_TYPE_CONTAINER', 1)
declare_const('WIDGET_TYPE_LIST', 2)
declare_const('WIDGET_TYPE_SELECT', 3)
declare_const('WIDGET_TYPE_DISPLAY', 4)
declare_const('WIDGET_TYPE_DISPLAY_STRING', 5)
declare_const('WIDGET_TYPE_EDIT', 6)
declare_const('WIDGET_TYPE_THREE_STATE_INDICATOR', 7)

#-------------------------------------------------------------------------------

COLOR_NAME_TO_HEX = {'aliceblue': 'F0F8FF', 'antiquewhite': 'FAEBD7', 'aquamarine': '7FFFD4', 'azure': 'F0FFFF', 'beige': 'F5F5DC', 'bisque': 'FFE4C4', 'black': '000000', 'blanchedalmond': 'FFEBCD', 'blue': '0000FF', 'blueviolet': '8A2BE2', 'brown': 'A52A2A', 'burlywood': 'DEB887', 'cadetblue': '5F9EA0', 'chartreuse': '7FFF00', 'chocolate': 'D2691E', 'coral': 'FF7F50', 'cornflowerblue': '6495ED', 'cornsilk': 'FFF8DC', 'cyan': '00FFFF', 'darkgoldenrod': 'B8860B', 'darkgreen': '006400', 'darkkhaki': 'BDB76B', 'darkolivegreen': '556B2F', 'darkorange': 'FF8C00', 'darkorchid': '9932CC', 'darksalmon': 'E9967A', 'darkseagreen': '8FBC8F', 'darkslateblue': '483D8B', 'darkslategray': '2F4F4F', 'darkturquoise': '00CED1', 'darkviolet': '9400D3', 'deeppink': 'FF1493', 'deepskyblue': '00BFFF', 'dimgray': '696969', 'dodgerblue': '1E90FF', 'firebrick': 'B22222', 'floralwhite': 'FFFAF0', 'forestgreen': '228B22', 'gainsboro': 'DCDCDC', 'ghostwhite': 'F8F8FF', 'gold': 'FFD700', 'goldenrod': 'DAA520', 'gray': '808080', 'green': '008000', 'greenyellow': 'ADFF2F', 'honeydew': 'F0FFF0', 'hotpink': 'FF69B4', 'indianred': 'CD5C5C', 'ivory': 'FFFFF0', 'khaki': 'F0E68C', 'lavender': 'E6E6FA', 'lavenderblush': 'FFF0F5', 'lawngreen': '7CFC00', 'lemonchiffon': 'FFFACD', 'lightblue': 'ADD8E6', 'lightcoral': 'F08080', 'lightcyan': 'E0FFFF', 'lightgoldenrod': 'EEDD82', 'lightgoldenrodyellow': 'FAFAD2', 'lightgray': 'D3D3D3', 'lightpink': 'FFB6C1', 'lightsalmon': 'FFA07A', 'lightseagreen': '20B2AA', 'lightskyblue': '87CEFA', 'lightslate': '8470FF', 'lightslategray': '778899', 'lightsteelblue': 'B0C4DE', 'lightyellow': 'FFFFE0', 'limegreen': '32CD32', 'linen': 'FAF0E6', 'magenta': 'FF00FF', 'maroon': 'B03060', 'mediumaquamarine': '66CDAA', 'mediumblue': '0000CD', 'mediumorchid': 'BA55D3', 'mediumpurple': '9370DB', 'mediumseagreen': '3CB371', 'mediumslateblue': '7B68EE', 'mediumspringgreen': '00FA9A', 'mediumturquoise': '48D1CC', 'mediumviolet': 'C71585', 'midnightblue': '191970', 'mintcream': 'F5FFFA', 'mistyrose': 'FFE4E1', 'moccasin': 'FFE4B5', 'navajowhite': 'FFDEAD', 'navy': '000080', 'oldlace': 'FDF5E6', 'olivedrab': '6B8E23', 'orange': 'FFA500', 'orangered': 'FF4500', 'orchid': 'DA70D6', 'palegoldenrod': 'EEE8AA', 'palegreen': '98FB98', 'paleturquoise': 'AFEEEE', 'palevioletred': 'DB7093', 'papayawhip': 'FFEFD5', 'peachpuff': 'FFDAB9', 'peru': 'CD853F', 'pink': 'FFC0CB', 'plum': 'DDA0DD', 'powderblue': 'B0E0E6', 'purple': 'A020F0', 'red': 'FF0000', 'rosybrown': 'BC8F8F', 'royalblue': '4169E1', 'saddlebrown': '8B4513', 'salmon': 'FA8072', 'sandybrown': 'F4A460', 'seagreen': '2E8B57', 'seashell': 'FFF5EE', 'sienna': 'A0522D', 'skyblue': '87CEEB', 'slateblue': '6A5ACD', 'slategray': '708090', 'snow': 'FFFAFA', 'springgreen': '00FF7F', 'steelblue': '4682B4', 'tan': 'D2B48C', 'thistle': 'D8BFD8', 'tomato': 'FF6347', 'turquoise': '40E0D0', 'violet': 'EE82EE', 'violetred': 'D02090', 'wheat': 'F5DEB3', 'white': 'FFFFFF', 'whitesmoke': 'F5F5F5', 'yellow': 'FFFF00', 'yellowgreen': '9ACD32'}

def HTMLColorToRGB(color_str):
    """ convert #RRGGBB to an (R, G, B) tuple """
    color_str = color_str.strip()
    if color_str[0] == '#': color_str = color_str[1:]
    if len(color_str) != 6:
        raise ValueError("input #%s is not in #RRGGBB format" % color_str)
    r, g, b = color_str[:2], color_str[2:4], color_str[4:]
    r, g, b = [int(n, 16) for n in (r, g, b)]
    return (r, g, b)

def color_exists(color_str):
    if color_str.lower() in COLOR_NAME_TO_HEX:
        return True

    try:
         HTMLColorToRGB(color_str)
    except ValueError as e:
        return False

    return True

def get_color(color_str):
    if color_str.lower() in COLOR_NAME_TO_HEX:
        r, g, b = HTMLColorToRGB(COLOR_NAME_TO_HEX[color_str.lower()])
    else:
        r, g, b = HTMLColorToRGB(color_str)
    # rrrrrggggggbbbbb
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b>>3)

#-------------------------------------------------------------------------------

DEFAULT_STYLE = {
    'font': DEFAULT_FONT,
    'flags': 0,
    'background_color': get_color('white'),
    'color': get_color('black'),
    'border_color': get_color('black'),
    'padding_horizontal': 0,
    'padding_vertical': 0
}

#-------------------------------------------------------------------------------

class Output:
    def __init__(self, single_indent='    '):
        self.single_indent = single_indent
        self.total_indent = ''
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

#-------------------------------------------------------------------------------

class Field:
    def __init__(self, name):
        self.name = name

    def enum_objects(self, objects):
        pass

    def finish(self):
        pass

class Struct(Field):
    def __init__(self, name, struct_name):
        super().__init__(name)
        self.fields = []
        self.struct_name = struct_name
        self.size = 2

    def addField(self, field):
        self.fields.append(field)

    def find_field(self, name):
        for field in self.fields:
            if field.name == name:
                return field

    def enum_objects(self, objects):
        for field in self.fields:
            field.enum_objects(objects)

    def finish(self):
        offset = 0
        for field in self.fields:
            field.offset = offset
            offset += field.size
        self.object_size = offset

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.object_offset)

    def pack_object(self):
        packed_data = []
        for field in self.fields:
            packed_data += field.pack()
        return packed_data

    def c_type(self):
        return 'OBJ_OFFSET'

class StructPtr(Field):
    def __init__(self, name, value):
        super().__init__(name)
        self.value = value
        self.size = 2

    def enum_objects(self, objects):
        if self.value:
            objects.append(self.value)

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.value.object_offset if self.value else 0)

    def c_type(self):
        return 'OBJ_OFFSET'

class StructWeakPtr(StructPtr):
    def __init__(self, name, value):
        super().__init__(name, value)

    def enum_objects(self, objects):
        pass

class List(Field):
    def __init__(self, name):
        super().__init__(name)
        self.items = []
        self.size = 4

    def addItem(self, item):
        self.items.append(item)

    def enum_objects(self, objects):
        for item in self.items:
            objects.append(item)

    def pack(self):
        return struct.pack(BYTE_ORDER + 'HH', len(self.items), self.items[0].object_offset if len(self.items) > 0 else 0)

    def c_type(self):
        return 'List'

class String(Field):
    def __init__(self, name, value):
        super().__init__(name)
        self.value = value
        self.size = 2
        self.object_size = len(self.value) + 1
        self.ap = False

    def enum_objects(self, objects):
        if not self.ap:
            objects.append(self)
            self.ap = True

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.object_offset)

    def pack_object(self):
        packed_data = (self.value + chr(0)).encode()
        return packed_data

    def c_type(self):
        return 'OBJ_OFFSET'

class UInt8(Field):
    def __init__(self, name, value):
        super().__init__(name)
        self.value = value
        self.size = 1

    def pack(self):
        return struct.pack(BYTE_ORDER + 'B', self.value)

    def c_type(self):
        return 'uint8_t'

class UInt16(Field):
    def __init__(self, name, value):
        super().__init__(name)
        self.value = value
        self.size = 2

    def pack(self):
        return struct.pack(BYTE_ORDER + 'H', self.value)

    def c_type(self):
        return 'uint16_t'
    
#-------------------------------------------------------------------------------

class Parser:
    def __init__(self, data_file_path, view_file_path):
        self.trace = Output()

        try:
            file = open(data_file_path)
        except Exception as e:
            self.trace.error("data file open error: %s" % e)
            return

        try:
            data = json.loads(file.read())
        except Exception as e:
            self.trace.error("data file format error: %s" % e)
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

        self.data = data
        self.view = view

        self.styles = {}

        self.parse_document()

    def num_errors(self):
        return self.trace.num_errors

    def num_warnings(self):
        return self.trace.num_warnings

    def addUInt16Field(self, struct, collection, name, default_value, mandatory=True, parent=None):
        if name in collection:
            value = collection[name]
            if not isinstance(value, int):
                self.trace.error(name + " is not an integer")
                value = default_value
        elif parent:
            value = parent.find_field(name).value
        else:
                if mandatory:
                    self.trace.error(name + " is missing")
                value = default_value
        struct.addField(UInt16(name, value))

    def addColorField(self, struct, collection, name, default_value, mandatory=True):
        if name in collection:
            color_str = collection[name]
            if color_exists(color_str):
                value = get_color(color_str)
            else:
                self.trace.error("%s: unknown color '%s'" % (name, color_str))
                value = default_value
        else:
            if mandatory:
                self.trace.error(name + " is missing")
            value = default_value
        struct.addField(UInt16(name, value))

    def addStringField(self, struct, collection, name, default_value, mandatory=True):
        if name in collection:
            value = collection[name]
            if not isinstance(value, str):
                self.trace.error(name + " is not an string")
                value = default_value
        else:
            if mandatory:
                self.trace.error(name + " is missing")
            value = default_value
        struct.addField(String(name, value))

    def parse_document(self):
        self.document = Struct('document', 'Document')

        styles = List('styles')
        for style in self.view['styles']:
            styles.addItem(self.parse_style(style))
        self.document.addField(styles)

        pages = List('pages')
        for page in self.view['pages']:
            pages.addItem(self.parse_page(page))
        self.document.addField(pages)

    def get_style(self, style_name):
        return self.styles.get(style_name.lower())

    def get_default_style(self):
        return self.get_style('default')

    def get_style_property(self, style, name):
        if style:
            return style.find_field(name).value
        default_style = self.get_default_style()
        if default_style:
            return default_style.find_field(name).value
        else:
            return DEFAULT_STYLE[name]

    def parse_style(self, style):
        result = Struct('style', 'Style')

        if not 'name' in style:
            self.trace.error("unnamed style found")
            return result

        self.trace.info("Style '%s'" % style['name'])

        self.trace.indent()

        if 'base' in style:
            default_style = self.get_style(style['base'])
            if not default_style:
                self.trace.error("unknown base style '%s'" % style['base'])
        else:
            default_style = None
        if not default_style:
            default_style = self.get_default_style()

        # font
        if 'font' in style:
            font_str = style['font'].lower()
            if font_str == 'small':
                font = SMALL_FONT
            elif font_str == 'medium':
                font = MEDIUM_FONT
            elif font_str == 'large':
                font = LARGE_FONT
            else:
                self.trace.error("unknown font '%s'" % style['font'])
                font = DEFAULT_FONT
        else:
            font = self.get_style_property(default_style, 'font')
        result.addField(UInt8('font', font))

        # flags
        flags = self.get_style_property(default_style, 'flags')
        if 'border_size' in style:
            flags &= ~STYLE_FLAGS_BORDER
            if style['border_size'] == 0:
                pass
            elif style['border_size'] == 1:
                flags |= STYLE_FLAGS_BORDER
            else:
                self.trace.error("only border_size 0 or 1 is supported")
        if 'align_horizontal' in  style:
            flags &= ~STYLE_FLAGS_HORZ_ALIGN
            align_str = style['align_horizontal'].lower()
            if align_str == 'left':
                pass
            elif align_str == 'right':
                flags |= STYLE_FLAGS_HORZ_ALIGN_RIGHT
            elif align_str == 'center':
                flags |= STYLE_FLAGS_HORZ_ALIGN_CENTER
            else:
                self.trace.error("only align_horizontal left, right or center is allowed")
        if 'align_vertical' in  style:
            flags &= ~STYLE_FLAGS_VERT_ALIGN
            align_str = style['align_vertical'].lower()
            if align_str == 'top':
                pass
            elif align_str == 'bottom':
                flags |= STYLE_FLAGS_VERT_ALIGN_BOTTOM
            elif align_str == 'center':
                flags |= STYLE_FLAGS_VERT_ALIGN_CENTER
            else:
                self.trace.error("only align_vertical top, bottom or center is allowed")
        result.addField(UInt16('flags', flags))

        self.addColorField(result, style, 'background_color', self.get_style_property(default_style, 'background_color'), False)
        self.addColorField(result, style, 'color', self.get_style_property(default_style, 'color'), False)
        self.addColorField(result, style, 'border_color', self.get_style_property(default_style, 'border_color'), False)

        self.addUInt16Field(result, style, 'padding_horizontal', self.get_style_property(default_style, 'padding_horizontal'), False)
        self.addUInt16Field(result, style, 'padding_vertical', self.get_style_property(default_style, 'padding_vertical'), False)

        self.trace.unindent()

        self.styles[style['name'].lower()] = result

        return result

    def parse_page(self, page):
        if 'name' in page:
            self.trace.info("Page '%s'" % page['name'])
        else:
            self.trace.info("Page '<noname>'")

        self.trace.indent()

        if not 'name' in page:
            self.trace.warning("unnamed page found")

        result = Struct('page', 'Page')

        self.addUInt16Field(result, page, 'w', 0)
        self.addUInt16Field(result, page, 'h', 0)

        if not 'widgets' in page:
            self.trace.error("widgets is missing")
        else:
            widgets = List('widgets')
            for index, widget in enumerate(page['widgets']):
                widgets.addItem(self.parse_widget(index, widget))
            result.addField(widgets)

        self.trace.unindent()

        return result

    def parse_widget(self, index, widget, parent=None):
        self.trace.info("Widget %d" % index)
        self.trace.indent()

        result = Struct('widget', 'Widget')

        if "type" in widget:
            type_str = widget['type'].lower()
            if type_str == 'container':
                widget_type = WIDGET_TYPE_CONTAINER
            elif type_str == 'list':
                widget_type = WIDGET_TYPE_LIST
            elif type_str == 'select':
                widget_type = WIDGET_TYPE_SELECT
            elif type_str == 'display':
                widget_type = WIDGET_TYPE_DISPLAY
            elif type_str == 'display_string':
                widget_type = WIDGET_TYPE_DISPLAY_STRING
            elif type_str == 'edit':
                widget_type = WIDGET_TYPE_EDIT
            elif type_str == 'three_state_indicator':
                widget_type = WIDGET_TYPE_THREE_STATE_INDICATOR
            else:
                self.trace.error("unkown type '%s'" % type_str)
                widget_type = WIDGET_TYPE_NONE
        else:
            self.trace.error("type is missing")
            widget_type = WIDGET_TYPE_NONE

        result.addField(UInt8("type", widget_type))

        if widget_type == WIDGET_TYPE_CONTAINER:
            # data is always 0
            result.addField(UInt16('data', 0))
        elif widget_type == WIDGET_TYPE_DISPLAY_STRING:
            # data is string
            self.addStringField(result, widget, 'data', "")
        else:
            self.addUInt16Field(result, widget, 'data', 0)

        self.addUInt16Field(result, widget, 'x', 0, parent=parent)
        self.addUInt16Field(result, widget, 'y', 0, parent=parent)
        self.addUInt16Field(result, widget, 'w', 0, parent=parent)
        self.addUInt16Field(result, widget, 'h', 0, parent=parent)

        if "style" in widget:
            style = self.get_style(widget['style'])
            if not style:
                self.trace.error("style '%s' not found" % widget['style'])
                style = self.get_default_style()
        else:
            style = self.get_default_style()
        result.addField(StructWeakPtr('style', style))

        if widget_type == WIDGET_TYPE_CONTAINER or widget_type == WIDGET_TYPE_LIST or widget_type == WIDGET_TYPE_SELECT:
            specific_widget_data = Struct(None, 'ContainerWidget')

            select_widgets = List('widgets')
            if 'widgets' in widget:
                for index, w in enumerate(widget['widgets']):
                    select_widgets.addItem(self.parse_widget(index, w, result))
            specific_widget_data.addField(select_widgets)
        elif widget_type == WIDGET_TYPE_THREE_STATE_INDICATOR:
            specific_widget_data = Struct(None, 'ThreeStateIndicatorWidget')

            if "style1" in widget:
                style1 = self.get_style(widget['style1'])
                if not style1:
                    self.trace.error("style1 '%s' not found" % widget['style1'])
                    style1 = self.get_default_style()
            specific_widget_data.addField(StructWeakPtr('style1', style1))

            if "style2" in widget:
                style2 = self.get_style(widget['style2'])
                if not style2:
                    self.trace.error("style2 '%s' not found" % widget['style2'])
                    style2 = self.get_default_style()
            specific_widget_data.addField(StructWeakPtr('style2', style2))

            self.addStringField(specific_widget_data, widget, 'text', "")
        else:
            specific_widget_data = 0

        result.addField(StructPtr("specific", specific_widget_data))

        self.trace.unindent()

        return result

#-------------------------------------------------------------------------------

def finish(document):
    objects = []

    current_objects = [document]
    while True:
        objects.extend(current_objects);
        new_objects = []
        for object in current_objects:
            object.enum_objects(new_objects)
        if len(new_objects) == 0:
            break
        current_objects = new_objects

    for object in objects:
        object.finish();
    
    object_offset = 0
    for object in objects:
        object.object_offset = object_offset
        object_offset += object.object_size

    return objects

def generate_source_code(objects, tab):
    result = '#pragma pack(push, 1)\n\n'

    #generate constants
    for constant in constants:
        result += '#define ' + constant['name'] + ' ' + str(constant['value']) + '\n'
    result += '\n'

    #
    result += 'typedef uint16_t OBJ_OFFSET;\n\n'

    result += 'struct List {\n'
    result += tab + 'uint16_t count;\n'
    result += tab + 'OBJ_OFFSET first;\n'
    result += '};\n\n'

    # generate structures
    structs = {}

    for object in objects:
        if isinstance(object, Struct):
            structs[object.struct_name] = object

    for struct in structs.values():
        result += 'struct ' + struct.struct_name + ' {\n'
        for field in struct.fields:
            result += tab + field.c_type() + ' ' + field.name + ';\n'
        result += '};\n\n'

    result += '#pragma pack(pop)\n\n'

    return result

def pack(objects):
    packed_data = []
    for object in objects:
        packed_data += object.pack_object()

    return packed_data

def to_c_buffer(self, tab, num_columns):
    c_buffer = '{'
    column_index = 0
    for byte in packed_data:
        if (column_index % num_columns) == 0:
            if column_index > 0:
                c_buffer += ','
            c_buffer += '\n' + tab
        else:
            c_buffer += ', '
        c_buffer += '0x%0.2X' % byte
        column_index += 1
    c_buffer += '\n}'
    return c_buffer
    
#-------------------------------------------------------------------------------

parser = Parser('./data.json', './view.json')
print("\n%d errors, %d warnings" % (parser.num_errors(), parser.num_warnings()))

if parser.num_errors() == 0:
    objects = finish(parser.document)

    if len(sys.argv) == 2:
        output_file = open(sys.argv[1], "w")
    else:
        output_file = sys.stdout

    output_file.write("""/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */\n\n""")

    c_structs = generate_source_code(objects, '    ')
    output_file.write(c_structs)

    packed_data = pack(objects)
    c_buffer = to_c_buffer(packed_data, '    ', 16)
    output_file.write('uint8_t document[%d] = %s;\n' % (len(packed_data), c_buffer))

    output_file.close()