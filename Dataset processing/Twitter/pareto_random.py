# -*- coding: utf-8 -*-
"""
Created on Sat Jan 23 21:17:30 2021

@author: 侯昌盛
"""

import codecs
import random
import math
source = codecs.open('twitter_aa','r','utf-8')
result = codecs.open('twitter_aa_1','w','utf-8')

alpha = 1.0
for line in source:
    tmp = random.paretovariate(alpha)
    tmp2 = math.ceil(tmp)
    s = line.split()
    result.write(s[0]+'\t'+s[1]+'\t'+str(tmp2)+'\r\n')

source.close()
result.close()
