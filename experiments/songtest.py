#!/usr/bin/env python3

from bs4 import BeautifulSoup 
import requests



url = 'https://zaccohn.com/misc/audio/'
ext = 'mp3'

page = requests.get(url).text
#print(page)
soup = BeautifulSoup(page, 'html.parser')
listOfmp3s = [url + node.get('href') for node in soup.find_all('a') if node.get('href').endswith(ext)]

names = []

for file in listOfmp3s:
	#print(file)
	name = file.rsplit("/",1)[1].rsplit(".",1)[0]
	print("test: " + name)
	names.append(name)