# GDB pretty-printers for TKernel types

import gdb.printing
import re

class OccTransientPrinter(gdb.ValuePrinter):
    """printer for Standard_Transient base class"""
    def __init__(self,val):
        return
    def to_string(self):
        return None

class OccHandlePrinter(gdb.ValuePrinter):
    """printer for opencascade::handle smart-pointer"""
    def __init__(self,val):
        entity = val['entity']
        self.child = []
        if entity == 0:
            self.output = str(entity)
        else:
            obj = entity.dereference()
            obj = obj.cast(obj.dynamic_type)
            self.output = str(entity) + ' : ' + str(obj['myRefCount_'])
            self.child.append((obj.type.tag, obj))
    def to_string(self):
        return self.output
    def children(self):
        return self.child
    def display_hint(self):
        return 'array'

class OccListPrinter(gdb.ValuePrinter):
    """printer for NCollection_List"""
    class _iterator(object):
        def __init__(self, list):
            listType = list.dynamic_type
            # handle NCollection_Shared<NCollection_List<...>>
            if re.match('^NCollection_Shared', listType.name) != None:
                listType = listType.template_argument(0)
            itemTypeName = listType.template_argument(0).name
            self.nodeType = gdb.lookup_type('NCollection_TListNode<%s>' % itemTypeName)
            self.current = list['myFirst']
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.current == 0:
                raise StopIteration
            next = self.current.dereference().cast(self.nodeType)
            self.current = next['myNext']
            count = self.count
            self.count = self.count + 1
            val = next['myValue']
            return ('[%d]' % count, val)

    def __init__(self, val):
        self.val = val
        self.size = val['myLength']
        self.typeName = val.dynamic_type.name

    def display_hint(self):
        return 'array'

    def to_string(self):
        if self.size == 0:
            return 'empty'
        return ('%s len %d' % (self.typeName, self.size))

    def children(self):
        return self._iterator(self.val)

class OccArray1Printer(gdb.ValuePrinter):
    """printer for NCollection_Array1"""

    class _iterator(object):
        def __init__ (self, array):
            arrType = array.dynamic_type
            # handle NCollection_Shared<NCollection_Array1<...>>
            if re.match('^NCollection_Shared', arrType.name) != None:
                arrType = arrType.template_argument(0)
            self.itemTypePtr = arrType.template_argument(0).pointer()
            self.data    = array['myData'].cast(self.itemTypePtr)
            self.currInd = array['myLowerBound']
            self.lastInd = array['myUpperBound']
            self.count   = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.currInd > self.lastInd:
                raise StopIteration
            self.count = self.count + 1
            item = (self.data + self.currInd).dereference()
            currInd = self.currInd
            self.currInd = self.currInd + 1
            return ('[%d]' % currInd, item)

    def __init__(self, val):
        self.val   = val
        self.lower = val['myLowerBound']
        self.upper = val['myUpperBound']
        self.typeName = val.dynamic_type.name

    def children(self):
        return self._iterator(self.val)

    def to_string(self):
        return ('%s [%d, %d]' % (self.typeName, self.lower, self.upper))

class OccDataMapPrinter(gdb.ValuePrinter):
    """printer for NCollection_DataMap"""

    class _iterator(object):
        def __init__(self, map):
            self.nbBuckets  = map['myNbBuckets']
            self.buckets    = map['myData1']
            self.currBucket = -1
            self.node       = 0
            if self.buckets == 0:
                self.nbBuckets = -1
            else:
                self.currBucket = self.currBucket + 1
                ifself.currBucket > self.nbBuckets:
                    return
                self.node = (self.buckets + self.currBucket).dereference()
                while self.node == 0:
                    self.currBucket = self.currBucket + 1
                    ifself.currBucket > self.nbBuckets:
                        return
                    self.node = (self.buckets + self.currBucket).dereference()
            self.count = 0
            self.nodeType = gdb.lookup_type('%s::DataMapNode' % map.dynamic_type.name).pointer()
            self.mapNode = None

        def __iter__(self):
            return self

        def __next__(self):
            if self.mapNode == None andself.node == 0:
                raise StopIteration
            if self.count % 2 == 0:
                self.mapNode = self.node.cast(self.nodeType).dereference()
                self.node = self.node.dereference()['myNext']
                while self.node == 0:
                    self.currBucket = self.currBucket + 1
                    if self.currBucket > self.nbBuckets:
                        break
                    self.node = (self.buckets + self.currBucket).dereference()
                item = self.mapNode['myValue']['first']
            else:
                item = self.mapNode['myValue']['second']
                self.mapNode = None
            result = ('[%d]' % self.count, item)
            self.count = self.count + 1
            return result

    def __init__(self, val):
        self.val = val
        self.size = val['mySize']
        self.typeName = val.dynamic_type.name

    def to_string(self):
        if self.size == 0:
            return 'empty'
        return ('%s len %d' % (self.typeName, self.size))

    def children (self):
        return self._iterator(self.val)

    def display_hint (self):
        return 'map'

class OccSequencePrinter(gdb.ValuePrinter):
    """printer for NCollection_Sequence"""

    class _iterator(object):
        def __init__(self, sequence):
            seqType = sequence.dynamic_type
            # handle NCollection_Shared<NCollection_Sequence<...>>
            if re.match('^NCollection_Shared', seqType.name) != None:
                seqType = seqType.template_argument(0)
            itemTypeName = seqType.template_argument(0).name
            self.nodeType = gdb.lookup_type('NCollection_Sequence<%s>::Node' %itemTypeName)
            self.current = sequence['myFirstItem']
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.current == 0:
                raise StopIteration
            currNode = self.current.dereference().cast(self.nodeType)
            self.current = currNode['myNext']
            count = self.count
            self.count = self.count + 1
            val = currNode['myValue']
            return ('[%d]' % count, val)

    def __init__(self, val):
        self.val  = val
        self.size = val['mySize']
        self.typeName = val.dynamic_type.name

    def display_hint(self):
        return 'array'

    def to_string(self):
        if self.size == 0:
            return 'empty'
        return ('%s len %d' % (self.typeName, self.size))

    def children(self):
        return self._iterator(self.val)

class OccVectorPrinter(gdb.ValuePrinter):
    """printer for NCollection_Vector"""

    class _iterator(object):
        def __init__ (self, vector):
            self.data = vector['myData']
            vecType = vector.dynamic_type
            # handle NCollection_Shared<NCollection_Vector<...>>
            if re.match('^NCollection_Shared', vecType.name) != None:
                vecType = vecType.template_argument(0)
            self.itemTypePtr = vecType.template_argument(0).pointer()
            nBlocks = vector['myNBlocks']
            if nBlocks == 0:
                self.endBlock = 0
                self.endInd = 0
                self.currBlock = 0
                self.currInd = 0
            else:
                self.endBlock = nBlocks - 1
                self.endInd = (self.data + self.endBlock).dereference()['Length']
                self.currBlock = 0
                self.currInd = 0
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.currBlock >= self.endBlock and self.currInd >= self.endInd:
                raise StopIteration
            if self.currInd >= (self.data + self.currBlock).dereference()['Length'] and self.currBlock < self.endBlock:
                self.currBlock = self.currBlock + 1
                self.currInd = 0
            count = self.count
            self.count = self.count + 1
            itemPtr = (self.data + self.currBlock).dereference()['DataPtr'].cast(self.itemTypePtr)
            item = (itemPtr + self.currInd).dereference()
            self.currInd = self.currInd + 1
            return ('[%d]' % count, item)

    def __init__(self,val):
        self.val      = val
        self.size     = val['myLength']
        self.inc      = val['myIncrement']
        self.capacity = val['myCapacity']
        self.typeName = val.dynamic_type.name

    def children(self):
        return self._iterator(self.val)

    def to_string(self):
        if self.size == 0:
            return 'empty'
        return ('%s len %d, inc %d, cap %d' % (self.typeName, self.size, self.inc, self.capacity))

    def display_hint(self):
        return 'array'

def build_tkernel_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("TKernel")
    pp.add_printer('OCC::Transient', 'Standard_Transient',         OccTransientPrinter)
    pp.add_printer('OCC::Handle',    '^opencascade::handle<.*>$',  OccHandlePrinter)
    pp.add_printer('OCC::Array1',    '^NCollection_Array1<.*>$',   OccArray1Printer)
    pp.add_printer('OCC::DataMap',   '^NCollection_DataMap<.*>$',  OccDataMapPrinter)
    pp.add_printer('OCC::List',      '^NCollection_List<.*>$',     OccListPrinter)
    pp.add_printer('OCC::Sequence',  '^NCollection_Sequence<.*>$', OccSequencePrinter)
    pp.add_printer('OCC::Vector',    '^NCollection_Vector<.*>$',   OccVectorPrinter)
    return pp

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_tkernel_printer())
