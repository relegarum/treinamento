#!/usr/bin/env python
import sys
import os
from multiprocessing import Process
import time

PORT  = "2196"
IP = "localhost"

download_file = '/index.html'
upload_file   = 'teste.tar.put'
file_urls = IP +':' + PORT + download_file
#file_urls = [IP +':' + PORT + '/pdfs/evolucaoDosCodecs.pdf',
#            IP + ':' + PORT + '/imgs/lena.jpeg',
#            IP +':' + PORT + '/imgs/mandrill.png',
#            IP +':' + PORT + '/teste_300mb.iso',
#            IP +':' + PORT + '/teste']
process_list = []

def uploadFile(index):

  print('Upload start --->' + str(index))
  os.system("curl " + IP + ":" + PORT + "/arquivos_download/" + upload_file + ".put_" + str(index) +
" --no-keepalive --progress-bar --upload-file " + upload_file )

def downloadFile(file_url, fileout_name,index): 
  print('Download start ---> '+ str(index)+ ' - ' + fileout_name)
  os.system('wget -nc  ' + file_url + ' -O arquivos_download/' 
    + fileout_name + "_" + str(index) )
  print('#### Download end ---> '+ str(index) +' - '+ fileout_name + " ") 
  os.system('cmp ' + fileout_name  + ' arquivos_download/' + fileout_name + '_' + str(index)) 
for i in range(0,20):
  print("-----------------------------------------------------")
  #if i % 2 == 0:
  if 0:
    p = Process(target=downloadFile, args=(file_urls,file_urls[file_urls.rfind('/')+1:],i, ))
    p.start()
  else:
    p = Process(target=uploadFile, args=(i, )).start()
  print("   #################  %d  ####################" % i)
#for i in xrange(len(file_urls)):
print("*******************************************************")
#  os.system("md5sum root/" + file_urls[i][file_urls[i].find('/')+1:]
 #   + " arquivos_download/" + file_urls[i][file_urls[i].rfind('/')+1:])
print("*******************************************************")

