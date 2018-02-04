#!/usr/bin/python3
from os.path import dirname, join, realpath, splitext
from subprocess import Popen, PIPE
from sys import executable as python, getdefaultencoding

executable=join(dirname(realpath(__file__)), 'beta' + splitext(python)[1])

class Sampler:
    def __init__(self, count):
        self._count = count
        self._child = Popen([executable, str(count)], stdin=PIPE, stdout=PIPE,
                            bufsize=1, universal_newlines=True,
                            encoding=getdefaultencoding())
    def __enter__(self):
        return self
    def __exit__(self, *details):
        self.close()

    def sample(self, pop, obs):
        print(pop, obs, file=self._child.stdin)
        for i in range(self._count):
            _pop, sam = self._child.stdout.readline().split()
            assert int(_pop) == pop
            yield sam

    def close(self):
        self._child.stdin.close()
        self._child.wait()
        del self._child

if __name__ == '__main__':
    DATA  = [(100,3), (20,10)]
    COUNT = 3

    # either terminate the subprocess manually:
    sam = Sampler(COUNT)
    for pop, obs in DATA:
        for i, val in enumerate(sam.sample(pop, obs)):
            # just emulate the output of binom(1) itself
            print(i, pop, val)
    sam.close()

    # or use the context manager:
    with Sampler(COUNT) as sam:
        for pop, obs in DATA:
            for i, val in enumerate(sam.sample(pop, obs)):
                print(i, pop, val)
