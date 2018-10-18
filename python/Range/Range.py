

class Range(object):
    def __init__(self, len_or_start, end=None, step=None):
        self.args = [len_or_start, end, step]
        self.start = 0
        self.end = len_or_start
        if end is not None:
            self.start = len_or_start
            self.end = end
        self.step = 1
        if step is not None:
            self.step = step
        if step == 0:
            raise ValueError("Range() arg 3 must not be zero")

        self.iterator_start = self.start

    def __repr__(self):
        if self.args[1] is None and self.args[2] is None:
            return "Range({0})".format(self.args[0])
        if self.args[2] is None:
            return "Range({0}, {1})".format(*self.args[:-1])
        return "Range({0}, {1}, {2})".format(*self.args)

    def in_range(self, item):
        if self.start > self.end:
            return self.end < item <= self.start
        return self.start <= item < self.end

    def __len__(self):
        if self.start == self.end:
            return 0
        if self.start < self.end and self.step < 0:
            return 0
        if self.start > self.end and self.step > 0:
            return 0
        numerator = abs(self.start - self.end) + abs(self.step) - 1
        return numerator // abs(self.step)

    def __contains__(self, item):
        if not self.in_range(item):
            return False
        mod = abs(item - self.start) % abs(self.step)
        if mod == 0:
            div = abs(item - self.start) // abs(self.step)
            new_item = self.start + div * self.step
            if item == new_item:
                return True
        return False

    def __getitem__(self, item):
        sz = len(self)
        if item >= sz:
            raise IndexError
        if item < -sz:
            raise IndexError
        if item < 0:
            item = sz + item
        elem_at = self.start + item * self.step
        return elem_at

    def __iter__(self):
        return Range(*self.args)

    def __next__(self):
        if not self.in_range(self.iterator_start):
            raise StopIteration
        self.iterator_start += self.step
        return self.iterator_start - self.step
