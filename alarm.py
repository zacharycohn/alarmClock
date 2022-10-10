#!/usr/bin/env python3


#imports
import sys
import time
from datetime import datetime, date, timedelta
import threading
import soco ## Sonos with python: https://github.com/SoCo/SoCo
import RGB1602

from bs4 import BeautifulSoup 
import requests



#Permanent Global Variables
alarmFileName = "alarmSettings.txt"
alarmFile = ""
alarmArmed = True
backlightStatus = True
setMode = 0
alarmTime = 0
alarmActive = False
maxVolume = 20
songNumber = 0
showTime = True


#Display Variables
colorR = 64
colorG = 128
colorB = 64
lcd=RGB1602.RGB1602(16,2)
lcd.setCursor(0, 0)
lcd.printout("Booting Up")

#rgbRed = (255,0,0)
lcd.setRGB(255,0,0);

class ShowClock(object):
	def __init__(self, interval=1):
		self.interval = interval

		thread = threading.Thread(target=self.run, args=())
		thread.daemon = True
		thread.start()

	def run(self):
		global alarmActive
		global showTime

		while True: 
			time.sleep(1)
			if alarmActive == False and showTime == True:
				updateDisplay("  The time is:  " + "      " + str(datetime.now().strftime("%H:%M")) + "     ")

class AlarmThreading(object):
	def __init__(self, interval=1):
		self.interval = interval

		thread = threading.Thread(target=self.run, args=())
		thread.daemon = True
		thread.start()

	def run(self):
		global alarmArmed
		global alarmActive
		global showTime

		while True: 
			time.sleep(1)

			if getTime() == readAlarm() and alarmArmed == True:
				showTime = False
				updateDisplay("Alarm!  Alarm!  Wake up! Alarm!")
				triggerSonos()
				alarmArmed = False
				reArm = DisarmThreading()


class DisarmThreading(object):
	def __init__(self, interval=1):
		self.interval = interval

		thread = threading.Thread(target=self.run, args=())
		thread.daemon = True
		thread.start()

		time.sleep(62)
		thread.join()
		defaultDisplay()

	def run(self):
		global alarmArmed
		global alarmActive

		#updateDisplay("Starting countdown")
		time.sleep(61)
		alarmArmed = True
		alarmActive = False




def main():
	global setMode
	global alarmTime
	global alarmActive
	global alarmArmed
	global alarmFile
	global showTime

	alarmTime = readAlarmFromFile()

	defaultDisplay()

	# updateDisplay("The time is " + getTime())
	# updateDisplay("Initial config: \n")
	# updateDisplay("Alarm set to: " + readAlarm())
	# setBacklightStatus()

	alarm = AlarmThreading()
	clock = ShowClock()

	cont = True
	while cont:

		text = input("Simulate input: ")

		if alarmActive == True and text != None: 
			pauseSonos()
			updateDisplay("Alarm silenced.")
			time.sleep(2)
			defaultDisplay()
		else:
			if text == "check alarm": #debug
				msg = "Alarm set to " + readAlarm() + " and is "
				if alarmArmed == True: 
					msg = msg + "armed"
				else:
					msg = msg + "not armed"
				updateDisplay(msg)
			elif text == "check time": #debug
				updateDisplay("The current time is " + getTime())
			elif text == "set":
				setMode = setMode + 1
				showTime = False

				if setMode == 1:
					try:
						alarmFile = open(alarmFileName, "w+")
					except:
						updateDisplay("alarm fail: " + str(sys.exc_info()[0]))
					updateDisplay("Set alarm time: ")
				elif setMode == 2:
					alarmFile.close()
					updateDisplay("Set max volume: ")
				elif setMode == 3: 
					songName = getSongName()
					updateDisplay("Current song:   " + songName)
				elif setMode == 4:
					updateDisplay("Press Arm to reset alarm.")
					response = input()
					if response == "arm alarm":
						resetAlarm()
						updateDisplay("Alarm reset to " + readAlarm())
					else:
						defaultDisplay()
						setMode = 0
				elif setMode == 5:
					defaultDisplay()
					setMode = 0

			elif text == "p": #right encoder up
				rightEncoderTurned(text)
			elif text == "l": # right encoder down
				rightEncoderTurned(text)
			elif text == "q": # left encoder up
				leftEncoderTurned(text)
			elif text == "a": #left encoder down
				leftEncoderTurned(text)
			elif text == "reset alarm":
				resetAlarm()
			elif text == "arm alarm":
				showTime = False
				switchArmedStatus()
				if alarmArmed == True:
					updateDisplay("Alarm: armed.")
					time.sleep(2)
					defaultDisplay()
				else:
					updateDisplay("Alarm: disarmed.")
					time.sleep(2)
					defaultDisplay()
			elif text == "backlight":
				setBacklightStatus()
			elif text == "exit":
				alarmFile.close()
				cont = False
			elif text == "find Sonos": #debug
				speaker = findSonos()
				updateDisplay("Speaker name is: " + speaker.player_name)
			elif text == "trigger Sonos": #debug
				triggerSonos()
			elif text == "pause Sonos": #debug
				pauseSonos()
			elif text == "help": #debug
				updateDisplay("List of commands: \n\n" + 
					"check time\n" + 
					"check alarm\n" +
					"set alarm\n" +
					"arm alarm\n" +
					"disarm alarm\n" +
					"backlight on\n" +
					"backlight off\n" +
					"find Sonos\n" + 
					"trigger Sonos\n" + 
					"pause Sonos\n" + 
					"test alarm\n" + 
					"exit\n"
					)
			else:
				showTime = False
				updateDisplay("pass")


def updateDisplay(content):
	str1 = ""
	str2 = ""

	str1 = content[0:16]
	str2 = content[16:]

	lcd.clear()

	lcd.setCursor(0,0)
	lcd.printout(str1)
	lcd.setCursor(0,1)
	lcd.printout(str2)

#	print(content)


def defaultDisplay():
	global showTime

	lcd.clear()
	lcd.setCursor(0,0)
	showTime = True



# def getSongName(): #hardcoded

# 	global songNumber

# 	title = []
# 	title.append("Daybreak")
# 	title.append("Make Your Own Kind of Music")
# 	title.append("Happy Morning")

# 	if songNumber >= len(title):
# 		songNumber = len(title) - 1
# 	elif songNumber < 0:
# 		songNumber = 0

# 	return title[songNumber]


# def getSongURL(): #hardcoded
# 	global songNumber

# 	track = []
# 	track.append("https://zaccohn.com/misc/audio/Michael%20Haggins%20-%20Daybreak.mp3")
# 	track.append("https://zaccohn.com/misc/audio/MamaCass.mp3")
# 	track.append("https://zaccohn.com/misc/audio/happymorning.mp3")

# 	if songNumber >= len(track):
# 		songNumber = len(track) - 1
# 	elif songNumber < 0:
# 		songNumber = 0


# 	return track[songNumber]


def getSongName():
	global songNumber

	url = 'https://zaccohn.com/misc/audio/alarm/'
	ext = 'mp3'

	page = requests.get(url).text
	#print(page)
	soup = BeautifulSoup(page, 'html.parser')
	listOfmp3s = [url + node.get('href') for node in soup.find_all('a') if node.get('href').endswith(ext)]

	title = []

	for file in listOfmp3s:
		name = file.rsplit("/",1)[1].rsplit(".",1)[0]
		title.append(name)

	if songNumber >= len(title):
		songNumber = 0
	elif songNumber < 0:
		songNumber = len(title) - 1

	return title[songNumber]


def getSongURL():
	# get a list from https://zaccohn.com/misc/audio/

	global songNumber

	url = 'https://zaccohn.com/misc/audio/alarm/'
	ext = 'mp3'

	page = requests.get(url).text
	#print(page)
	soup = BeautifulSoup(page, 'html.parser')
	track = [url + node.get('href') for node in soup.find_all('a') if node.get('href').endswith(ext)]


	if songNumber >= len(track):
		songNumber = 0
	elif songNumber < 0:
		songNumber = len(track) - 1

	return track[songNumber]



def changeSong(value):
	global songNumber

	songNumber = songNumber + value



def findSonos():
	alarmSpeakerName = "Bedroom"
	speaker = "error"
	speakers = soco.discover()


	for val in speakers:
		if str(val.player_name) == alarmSpeakerName:
			speaker = val
			break

	return speaker

def triggerSonos():

	global maxVolume
	global alarmActive 
	songName = getSongName()
	songURL = getSongURL()

	speaker = findSonos()
	speaker.unjoin()

#	speaker.pause()
#	queue = speaker.get_queue()
#	queue = speaker.music_library.list_library_shares()
	#updateDisplay(queue)

	alarmActive = True

	vol = 1
	speaker.volume = vol 

	speaker.play_uri(songURL)
#	track = speaker.get_current_track_info()


#	updateDisplay("Activating " + speaker.player_name + " to play " + songName) #track['title'])
	#updateDisplay("194:" + str(alarmActive))

	while vol < maxVolume and alarmActive == True:
		#vol = speaker.volume
		#updateDisplay("Vol: " + str(vol))
		speaker.volume = vol + 1
		time.sleep(2)
		vol = vol + 1

#	time.sleep(30)
#	speaker.pause()
#	speaker.play()

def pauseSonos():
	global alarmActive

	speaker = findSonos()
	speaker.unjoin()
	speaker.stop()

	alarmActive = False


def leftEncoderTurned(adjustment):
	encoderTurned("hours", adjustment)


def rightEncoderTurned(adjustment):	
	encoderTurned("minutes", adjustment)


def encoderTurned(encoder, adjustment):
	value = 0

#	if "q" in adjustment or "p" in adjustment:
	if adjustment == "q" or adjustment == "p":
		value = 1
	elif adjustment == "a" or adjustment == "l":
#	elif "a" in adjustment or "l" in adjustment:
		value = -1


	if setMode == 1:
		setAlarm(encoder, value)
		updateDisplay("Alarm set: " + readAlarm() + "Hit set to save.")
	elif setMode == 2:
		setMaxVolume(value)
		updateDisplay("Max volume: " + str(maxVolume) + "  Hit set to save.")
	elif setMode == 3:
		changeSong(value)
		songName = getSongName()
		updateDisplay("Alarm: " + songName)
	else:
		pass


def setAlarm(component, value):

	global alarmTime
	global alarmFile

	## maybe this just changes a variable and after 5 seconds of no activity writes to the file? 
	time_format_str = '%H:%M'
	try:
		alarmTime = readAlarm()
		alarmTime = datetime.strptime(alarmTime, time_format_str)
	except: 
		#updateDisplay(str(sys.exc_info()[0]))
		updateDisplay("Error 423: Resetting alarm to default time.")
		time.sleep(2)
		resetAlarm()
		alarmTime = readAlarm()
		alarmTime = datetime.strptime(alarmTime, time_format_str)

	if component == "minutes":
		newAlarmTime = alarmTime + timedelta(minutes=value)
	elif component == "hours":
		newAlarmTime = alarmTime + timedelta(hours=value)

	alarm_set_time = str(date.strftime(newAlarmTime, time_format_str))

	alarmTime = alarm_set_time

#	alarmFile = open(alarmFileName, "w")
	alarmFile.seek(0)
	alarmFile.write(alarm_set_time)
#	alarmFile.close()



def readAlarm():
	global alarmTime
	return alarmTime



def readAlarmFromFile():

	global alarmFile
	time_format_str = '%H:%M'

	try: 
		alarmFile = open(alarmFileName,"r")
		alarmTime = alarmFile.read()
		junk = datetime.strptime(alarmTime, time_format_str) #to see if it fails
		
		#alarmTime = alarmFile.read()

#		updateDisplay("460: " + alarmTime)

	except:

#		updateDisplay(str(sys.exc_info()[0]))
		updateDisplay("Error 462: Resetting alarm to default time.")
		resetAlarm()
		time.sleep(2)
		alarmFile = open(alarmFileName,"r")
		alarmTime = alarmFile.read()
		#alarmTime = str(datetime.strptime(alarmTime, time_format_str))
#		updateDisplay("470: " + alarmTime)


	alarmFile.close()

	return alarmTime.rstrip()

def resetAlarm():
	global alarmTime

	#for testing purposes only or to recover from a crash

	alarm_reset_time = '07:33'

	alarmFile = open(alarmFileName, "w")
	alarmFile.write(alarm_reset_time)
	alarmFile.close()

	
	alarmTime = alarm_reset_time	

#	updateDisplay("Alarm is reset to: " + readAlarm())

def getTime():
	currentTime = time.localtime()
	currentTime = time.strftime("%H:%M",currentTime)

	return currentTime

def setBacklightStatus():
	global backlightStatus

	if backlightStatus == True:
		backlightStatus = False
		lcd.setRGB(0,0,0)

		#setupdateDisplay("backlight is now off")
	elif backlightStatus == False:
		backlightStatus = True
		lcd.setRGB(255,0,0)
		#updateDisplay("backlight is now on")


def switchArmedStatus():
	global alarmArmed

	if alarmArmed == True:
		alarmArmed = False
	else:
		alarmArmed = True


def setMaxVolume(adjustment):
	global maxVolume

	maxVolume = maxVolume + adjustment
	if maxVolume < 0:
		maxVolume = 0

	if maxVolume > 50:
		maxVolume = 50

	#updateDisplay("Max volume: " + str(maxVolume))


if __name__ == "__main__":
    main()