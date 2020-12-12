import unittest
import pyslidingwindow
try:
    import numpy
except ImportError:
    numpy = None

class TestWindow(unittest.TestCase):
    MAX = 10

    def test_empty(self):
        w = pyslidingwindow.SlidingWindowInt(1)

        # CXX interface
        self.assertEqual(w.length(),0)
        self.assertEqual(w.max_len(),1)
        with self.assertRaises(RuntimeError):
            w.get(0)

        # Python interface
        self.assertEqual(len(w),0)
        with self.assertRaises(RuntimeError):
            w[0]

        return

    def test_non_iter_arg(self):
        "optional second arg must be iterable"
        with self.assertRaises(RuntimeError):
            w = pyslidingwindow.SlidingWindowInt(self.MAX,0)
        return

    def test_chained_push(self):
        w = pyslidingwindow.SlidingWindowInt(self.MAX)
        w.push(1).push(2).push(3)
        self.assertEqual(list(w),[1,2,3])
        return

    def test_partial(self):
        w = pyslidingwindow.SlidingWindowInt(self.MAX)

        # CXX interface
        self.assertEqual(w.length(),0)
        self.assertEqual(w.max_len(),self.MAX)

        # Nothing in it (should raise)
        with self.assertRaises(RuntimeError):
            w[0]

        # If we add one item, it is the first (0'th) and last item (-1'th)
        w.push(11)
        self.assertEqual(len(w),1)
        self.assertEqual(w.max_len(),self.MAX)
        self.assertEqual(w[0],11)
        self.assertEqual(w[-1],11)
        self.assertEqual(w.first(),11)
        self.assertEqual(w.last(),11)

        # If we add another item, we get a new last
        w.push(22)
        self.assertEqual(len(w), 2)
        self.assertEqual(w.max_len(),self.MAX)
        self.assertEqual(w[0],11)
        self.assertEqual(w[-1],22)
        self.assertEqual(w.first(),11)
        self.assertEqual(w.last(),22)

        # If we fill the buffer (but don't wrap it), the first is still 11, but the last should be 70
        space = w.max_len() - len(w)
        for i in range(space):
            w.push((i+1)*10)  # 10, 20, ..., 70
        self.assertEqual(w[0],11)
        self.assertEqual(w[-1],80)
        self.assertEqual(w.first(),11)
        self.assertEqual(w.last(),80)
        fetched = [w[i] for i in range(self.MAX)]
        self.assertEqual(fetched,[11,22,10,20,30,40,50,60,70,80])

        return

    def test_optional_fill_arg(self):
        # We can use a second, optional arg (an iter item) to prefill the window
        # Here we test a straight iterator
        w = pyslidingwindow.SlidingWindowInt(self.MAX,(i+1 for i in range(self.MAX)))
        expected = list(range(1,self.MAX+1))
        fetched = [w[i] for i in range(self.MAX)]
        self.assertEqual(fetched,expected)

        # Or we can use an object that supports iteration
        w2 = pyslidingwindow.SlidingWindowInt(self.MAX,expected)
        fetched = [w[i] for i in range(self.MAX)]
        self.assertEqual(fetched,expected)

        return

    def test_iter(self):
        expected = list(range(self.MAX))
        w = pyslidingwindow.SlidingWindowInt(self.MAX,expected)

        # If we iterate across the window, we should get the same thing back
        fetched = list(w)
        self.assertEqual(fetched,expected)

        # If we add one item past the limit, we should get that in the first slot
        w.push(111)
        fetched = list(w)
        del expected[0]
        expected.append(111)
        self.assertEqual(fetched,expected)

        return


    def test_raw(self):
        # Only test if available to use
        w = pyslidingwindow.SlidingWindowInt(self.MAX)
        try:
            w.raw()
        except NotImplementedError:
            return

        # With nothing pushed, we should get a size 0 array of the right type, length, dimensionality
        a0 = w.raw()
        self.assertIsInstance(a0,numpy.ndarray)
        self.assertEqual(len(a0),0)
        self.assertEqual(a0.shape,(0,))

        # Push an item and we should see it in the array
        w.push(11)
        a1 = w.raw()
        self.assertIsInstance(a1,numpy.ndarray)
        self.assertEqual(len(a1),1)
        self.assertEqual(a1.shape,(1,))
        self.assertIn(11,a1)

        # Push until we overflow the ring (which should eject the 11 we just pushed)
        for i in range(self.MAX):
            w.push(i)
        self.assertEqual(len(w),self.MAX)

        # Grab the new raw array view of the window
        a10 = w.raw()
        self.assertIsInstance(a10,numpy.ndarray)
        self.assertEqual(len(a10),self.MAX)
        self.assertEqual(a10.shape,(self.MAX,))
        self.assertNotIn(11,a10)

        # We expect to see this to look like [9,0,1,..8]
        expected = list(range(self.MAX))
        expected = expected[-1:] + expected[:-1]
        self.assertEqual(a10.tolist(),expected)

        return

if __name__ == '__main__':
    unittest.main()
