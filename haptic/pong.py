#/usr/bin/env python
"""
Pong game borrowed from https://github.com/furbiesandbeans/PyPong with some slight adjustments
"""

import os, pygame, time, sys, pickle, random
from pygame.locals import *

pygame.init()

DOWNRIGHT = 3
UPRIGHT = 9
UPLEFT = 7
DOWNLEFT = 1

GAME_ENDED = 0
WINNING_PLAYER = 0

WHEIGHT = 600
WWIDTH = 800

#Color definitions
WHITE = (255, 255, 255)
GREEN = (0, 255, 0)
BLACK = (0 , 0, 0)
RED = (200, 0, 0)

PLAYERSPEED = 7
numOfPlayers = 0

basicFont = pygame.font.SysFont(None, 48)
smallFont = pygame.font.SysFont(None, 24)

#Make global to facilitate coding
windowSurface = pygame.display.set_mode((WWIDTH,WHEIGHT), 0, 32)
player1 = 0
ball = 0

random.seed()

#functions to create our resources
#Taken from: http://www.pygame.org/docs/tut/chimp/chimp.py.html
def load_image(name, colorkey=None):
	fullname = os.path.join('images', name)
	try:
		image = pygame.image.load(fullname)
	except pygame.error, message:
		print 'Cannot load image:', fullname
		raise SystemExit, message
	image = image.convert()
	if colorkey is not None:
		if colorkey is -1:
			colorkey = image.get_at((0,0))
		image.set_colorkey(colorkey, RLEACCEL)
	return image, image.get_rect()

def load_sound(name):
	class NoneSound:
		def play(self): pass
	if not pygame.mixer or not pygame.mixer.get_init():
		return NoneSound()
	fullname = os.path.join('sounds', name)
	try:
		sound = pygame.mixer.Sound(fullname)
	except pygame.error, message:
		print 'Cannot load sound:', fullname
		raise SystemExit, message
	return sound

#Define the ball class
class Ball(pygame.sprite.Sprite):
	# The ball class
	def __init__(self):
		pygame.sprite.Sprite.__init__(self) #call Sprite initializer
		self.image, self.rect = load_image('ball.bmp', -1)
		self.LRspeed = 5
		self.dir = DOWNRIGHT
		self.UDspeed = 1
		self.rect.topleft = 100,100
		self.pong_sound = load_sound('pong.wav')
	
	def update(self):
		global GAME_ENDED
		global WINNING_PLAYER
		if self.dir == DOWNRIGHT:
			self.rect.move_ip(self.LRspeed, self.UDspeed)
		elif self.dir == UPRIGHT:
			self.rect.move_ip(self.LRspeed, self.UDspeed)
		elif self.dir == UPLEFT:
			self.rect.move_ip(-self.LRspeed, self.UDspeed)
		elif self.dir == DOWNLEFT:
			self.rect.move_ip(-self.LRspeed, self.UDspeed)
		
		if self.rect.top < 0:
			if self.dir == UPRIGHT:
				self.dir = DOWNRIGHT
			elif self.dir == UPLEFT:
				self.dir = DOWNLEFT
			self.UDspeed = -self.UDspeed
			self.pong_sound.play()
		elif self.rect.bottom > WHEIGHT:
			if self.dir == DOWNRIGHT:
				self.dir = UPRIGHT
			elif self.dir == DOWNLEFT:
				self.dir = UPLEFT
			self.UDspeed = -self.UDspeed
			self.pong_sound.play()
		elif self.rect.right > WWIDTH and numOfPlayers == 1: #Added for testing single player
			if self.dir == UPRIGHT:
				self.dir = UPLEFT
			elif self.dir == DOWNRIGHT:
				self.dir = DOWNLEFT
			self.LRspeed = self.LRspeed + .5
			
			#Beyond 29 the ball doesn't bounce off players
			if self.LRspeed > 29:
				self.LRspeed = 29
			self.pong_sound.play()

		if self.rect.left < 0:
			GAME_ENDED = 1
			WINNING_PLAYER = 2
		elif self.rect.right > WWIDTH and numOfPlayers == 2:
			GAME_ENDED = 1
			WINNING_PLAYER = 1
			
	def collided(self):
		
		if self.UDspeed < 0:
			if self.dir == DOWNRIGHT or self.dir == UPRIGHT:
				self.dir = UPLEFT
			else:
				self.dir = UPRIGHT
		else:
			if self.dir == DOWNRIGHT or self.dir == UPRIGHT:
				self.dir = DOWNLEFT
			else:
				self.dir = DOWNRIGHT
		if numOfPlayers == 2:
			self.LRspeed = self.LRspeed + .5	
		else:
			self.LRspeed = self.LRspeed + .25
		#Beyond 29 the ball doesn't bounce off players
		if self.LRspeed > 29:
			self.LRspeed = 29
		
	
	def setSpeed(self, speed):
		self.UDspeed = speed


#End of class Ball


class Player(pygame.sprite.Sprite):
	# The Player class 
	def __init__(self):
		pygame.sprite.Sprite.__init__(self) #call Sprite initializer
		self.image, self.rect = load_image('player.bmp', GREEN)
			
	def set(self, num):
		if num == 1:
			self.rect.topleft = 30, 150
		elif num == 2:
			self.image = pygame.transform.flip(self.image, 1, 0)
			self.rect.topright = WWIDTH - 30, 150
		else:
			print 'GAME PLAYER NOT FOUND'
			pygame.quit()
			sys.exit()
		
	def update(self):
		nothing = 1
		
	def collidedAt(self, object):
		return ((object.rect.centery - self.rect.centery) / 5)
	
	def move(self, speed):
		newpos = self.rect.move(0,speed)
		if newpos.top > 0 and newpos.bottom < WHEIGHT:
			self.rect = newpos


		
# Function to create simple text
def printText(msg, surface, color, x, y, choice):
	if( choice == 1 ):
		text = basicFont.render(msg, True, color)
	else:
		text = smallFont.render(msg, True, color)
	textRect = text.get_rect()
	textRect.centerx = x
	textRect.centery = y
	surface.blit(text,textRect)


#def num_of_players_menu():


def main():
	
	#initialize everything
	global numOfPlayers
	global windowSurface
	pygame.display.set_caption('PyPong!')
	
	#Load player bounce sound
	ping_sound = load_sound('ping.wav')
	
	#Loading Ball
	global ball
	ballSpeed = 1
	ball = Ball()
	ball.setSpeed(ballSpeed)
	
	#Loading Players
	global player1
	player1 = Player()
	player1.set(1)
	player2 = Player()
	player2.set(2)
	
	#Loading boss
	
	#Load the highscore
	highScore = 0
	
	#For setting the FPS
	clock = pygame.time.Clock()
	
	temp = 1
	numOfPlayers = 2
	while temp:
		for event in pygame.event.get():
			if event.type == QUIT:
				pygame.quit()
				sys.exit()
			if event.type == pygame.KEYUP:
				temp = 0
		windowSurface.fill(WHITE)
		origStart = 10
		printText('Player 1 Controls: ', windowSurface, RED, 100, origStart, 0)
		printText('   Up: W Key', windowSurface, RED, 100, origStart + 20,  0)
		printText('   Down: S Key', windowSurface, RED, 100, origStart + 40, 0)
		printText('Player 2 Controls: ', windowSurface, RED, WWIDTH - 100 , WHEIGHT - 80, 0)
		printText('   Up: Up Key', windowSurface, RED, WWIDTH - 100 , WHEIGHT - 60, 0)
		printText('   Down: Down Key', windowSurface, RED, WWIDTH - 100 , WHEIGHT - 40, 0)
		printText('Press any key to start...' , windowSurface, RED, WWIDTH/2, WHEIGHT/2, 1)
		pygame.display.update()
		
		sprites = pygame.sprite.RenderPlain((ball,player1, player2))
		while not GAME_ENDED:
			temp = 0
			clock.tick(60)
			for event in pygame.event.get():
				if event.type == QUIT:
					pygame.quit()
					sys.exit()
	
			# Player Controls
			# Player 1 (Left):
			#		Up: W
			#		Down: S
			# Player 2 (right):
			#		Up: Up key
			#		Down: Down key
			key_press = pygame.key.get_pressed() #get keys pressed down
			if key_press[K_UP]:
				player2.move(-PLAYERSPEED)
			if key_press[K_DOWN]:
				player2.move(PLAYERSPEED)
			if key_press[K_w]:
				player1.move(-PLAYERSPEED)
			if key_press[K_s]:
				player1.move(PLAYERSPEED)		
			
			if player1.rect.colliderect(ball.rect):
				ballSpeed = player1.collidedAt(ball)
				ball.setSpeed(ballSpeed)
				ball.collided()
				ping_sound.play()
			elif player2.rect.colliderect(ball.rect):
				ballSpeed = player2.collidedAt(ball)
				ball.setSpeed(ballSpeed)
				ball.collided()
				ping_sound.play()
				
			windowSurface.fill(WHITE)
			sprites.update()
			sprites.draw(windowSurface)
			pygame.display.update()
			#time.sleep(.002)
	
		#Game ended, display winning player
		
		printText('Game Over!', windowSurface, BLACK, windowSurface.get_rect().centerx, 200, 1)
		printText('Player %d WON!!!' % WINNING_PLAYER, windowSurface, RED, windowSurface.get_rect().centerx, 250, 1)
		printText('Press Q or ESC to quit...', windowSurface, BLACK, windowSurface.get_rect().centerx, 290, 0)
	
		pygame.display.update()
	
	#Don't end until Q, ESC, or explicit quit
	while 1:
		for event in pygame.event.get():
			if event.type == QUIT:
				pygame.quit()
				sys.exit()
			if event.type == KEYDOWN:
				if event.key == K_q or event.key == K_ESCAPE:
					pygame.quit()
					sys.exit()					

if __name__ == '__main__': main()
		
