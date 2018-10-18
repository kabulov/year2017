class myproperty(object):
    def __init__(self, getter, setter):      
        self.getter = getter
        self.setter = setter
    
    def __set__(self, instance, value):
        if self.setter is None:
            raise AttributeError
        self.setter(instance, value)
        
    def __get__(self, instance, owner):
        if self.getter is None:
            raise AttributeError
        return self.getter(instance)


class PropertyMaker(type):
    def __init__(cls, name, bases, attrs, **kwargs):
        prname = set()
        setter = {}
        getter = {}        
        for elem in attrs:  
            if elem.startswith("get_"):
                name = elem[4:]
                prname.add(name)
                getter[name] = getattr(cls, elem)
            elif elem.startswith("set_"):                
                name = elem[4:]
                prname.add(name)
                setter[name] = getattr(cls, elem)
        
        for elem in prname:
            gttr = None
            sttr = None
            if elem in setter:
                sttr = setter[elem]
            if elem in getter:
                gttr = getter[elem]
            prop = myproperty(gttr, sttr)
            setattr(cls, elem, prop)
            setattr(cls, "_"+elem, 0)
            
        return super().__init__(name, bases, attrs)
