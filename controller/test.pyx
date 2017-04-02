#coding=utf-8

from helper cimport *

import unittest
import matplotlib.pyplot as plt
import numpy as np

class MLS_Test(unittest.TestCase):
    def test_mls(self):
        cdef MovingLeaseSquare mls
        cdef uint16_t buffer_time[50]
        cdef int16_t buffer_value[50]

        a = 2.0
        b = 0.0
        
        T = np.arange(100)
        X = a * T + b

        est_list = []
        MLS_Init(&mls, 50, buffer_time, buffer_value, 50)
        for i in xrange(len(T)):
            MLS_Append(&mls, <uint16_t>T[i], <int16_t>X[i])
            est_list.append(mls.current_value)

        plt.plot(T, est_list, 'b-')
        plt.plot(T, X, 'r+')
        plt.show()

