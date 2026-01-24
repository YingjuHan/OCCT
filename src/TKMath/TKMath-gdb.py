# GDB pretty-printers for TKMath types

import gdb.printing

class OccFullPreciParam(gdb.Parameter):
    """Tells double-precision type members to be printed at full precision"""
    def __init__(self):
        super(OccFullPreciParam, self).__init__("fullprecision", gdb.COMMAND_DATA,
                                                gdb.PARAM_BOOLEAN)
        self.value = False
    def get_set_string(self):
        value_string = "off"
        if self.value:
            value_string = "on"
        return "OCCT coordinates at full precision: " + value_string
    def get_show_string(self, value_string):
        return "OCCT coordinates at full precision: " + value_string

# Returns floating-point print format based on 'fullprecision' GDB parameter.
def get_float_fmt():
    if gdb.parameter("fullprecision"):
        return "%.15g"
    else:
        return "%9.7g"

# Print two floats
def print_vec2(x, y):
    fmt = get_float_fmt()
    return (fmt + ' ' + fmt) % (x, y)

# Print three floats
def print_vec3(x, y, z):
    fmt = get_float_fmt()
    return (fmt + ' ' + fmt + '  ' + fmt) % (x, y, z)

class OccGpXyPrinter(gdb.ValuePrinter):
    """printer for gp_XY"""
    def __init__(self, val):
        self.x = val['x']
        self.y = val['y']
    def to_string(self):
        return print_vec2(self.x, self.y)

class OccGpPnt2dPrinter(gdb.ValuePrinter):
    """printer for gp_Pnt2d, gp_Vec2d, gp_Dir2d"""
    def __init__(self, val):
        coord = val['coord']
        self.x = coord['x']
        self.y = coord['y']
    def to_string(self):
        return print_vec2(self.x, self.y)

class OccGpXyzPrinter(gdb.ValuePrinter):
    """printer for gp_XYZ"""
    def __init__(self, val):
        self.x = val['x']
        self.y = val['y']
        self.z = val['z']
    def to_string(self):
        return print_vec3(self.x, self.y, self.z)

class OccGpPntPrinter(gdb.ValuePrinter):
    """printer for gp_Pnt, gp_Vec, gp_Dir"""
    def __init__(self, val):
        coord = val['coord']
        self.x = coord['x']
        self.y = coord['y']
        self.z = coord['z']
    def to_string(self):
        return print_vec3(self.x, self.y, self.z)

class OccPolyTrianglePrinter(gdb.ValuePrinter):
    """printer for Poly_Triangle"""
    def __init__(self, val):
        self.nodes = val['myNodes']
    def to_string(self):
        return '%d %d %d' % (self.nodes[0], self.nodes[1], self.nodes[2])

def build_tkmath_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("TKMath")
    pp.add_printer('OCC::gp_XY',         '^gp_XY$',              OccGpXyPrinter)
    pp.add_printer('OCC::gp_XYZ',        '^gp_XYZ$',             OccGpXyzPrinter)
    pp.add_printer('OCC::gp_Pnt',        '^gp_(Pnt|Vec|Dir)$',   OccGpPntPrinter)
    pp.add_printer('OCC::gp_Pnt2d',      '^gp_(Pnt|Vec|Dir)2d$', OccGpPnt2dPrinter)
    pp.add_printer('OCC::Poly_Triangle', '^Poly_Triangle$',      OccPolyTrianglePrinter)
    return pp

OccFullPreciParam()

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_tkmath_printer())
