# -*- coding: utf-8 -*-
"""
Created on Fri Jan 22 08:15:48 2021

@author: 侯昌盛
"""

import codecs
source = codecs.open('authors_encoded.txt','r','utf-8')
result = codecs.open('authors_pair.txt','w','utf-8')

for line in source:
    tmp = []
    name_id_list = line.split(',')
    if len(name_id_list) > 2:
        for name_id in name_id_list:
            if not (name_id == '\r\n'):
                tmp.append(name_id)
        for i in range(len(tmp)):
            for j in range(len(tmp)):
                if i != j:
                    result.write(tmp[i]+' '+tmp[j])
                    result.write('\r\n')

source.close()
result.close()