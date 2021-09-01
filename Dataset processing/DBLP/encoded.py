# -*- coding: utf-8 -*-
"""
Created on Fri Jan 22 08:06:38 2021

@author: 侯昌盛
"""

import codecs
source = codecs.open('authors.txt','r','utf-8')
result = codecs.open('authors_encoded.txt','w','utf-8')
index = codecs.open('authors_index.txt','w','utf-8')
index_dic = {}
name_id = 0
## build an index_dic, key -> authorName value => [id, count]
for line in source:
    name_list = line.split(',')
    for name in name_list:
        if not (name == '\r\n'):
            if name in index_dic:
                index_dic[name][1] +=1
            else:
                index_dic[name] = [name_id,1]
                index.write(name + u'\r\n')
                name_id += 1
            result.write(str(index_dic[name][0]) + u',')
    result.write('\r\n')

source.close()
result.close()
index.close()