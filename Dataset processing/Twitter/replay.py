# -*- coding: utf-8 -*-
"""
Created on Fri Jan 22 12:05:50 2021

@author: 侯昌盛
"""

import codecs
import random
source = codecs.open('twitter_24000000','r','utf-8')
result = codecs.open('twitter_24000000_r','w','utf-8')

for line in source:
    tmp = random.randint(1, 9)
    s = line.split()
    for i in range(tmp):
        result.write(s[0]+'\t'+s[1]+'\r\n')

source.close()
result.close()