/*
PROGRAM: Brick Breakers Using SDL
AUTHOR: Patrick Malara
DATE: November 8, 2015
*/

//Including All of the cool stuff
#include <SDL.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <SDL_image.h>

//Screen Dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int JOYSTICK_DEAD_ZONE = 8000;

//Main loop flag
bool isRunning = true;

bool enemyShoot = false;

//Will be use later
enum GAMEMODE{
	MENU,
	PLAY,
	SCORE
};

//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

#ifdef _SDL_TTF_H
	//Creates image from font string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
#endif

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

//The application time based timer
class LTimer
{
public:
	//Initializes variables
	LTimer();

	//The various clock actions
	void start();
	void stop();
	void pause();
	void unpause();

	//Gets the timer's time
	Uint32 getTicks();

	//Checks the status of the timer
	bool isStarted();
	bool isPaused();

private:
	//The clock time when the timer started
	Uint32 mStartTicks;

	//The ticks stored when the timer was paused
	Uint32 mPausedTicks;

	//The timer status
	bool mPaused;
	bool mStarted;
};

//The player that will move around on the screen
class Player
{
public:
	//The dimensions of the player
	static const int PLAYER_WIDTH = 80;
	static const int PLAYER_HEIGHT = 40;

	//Maximum axis velocity of the dot
	static const int PLAYER_VEL = 9;

	unsigned int score;

	unsigned int healthPoints;
	unsigned int lives;

	//Initializes the variables
	Player();

	//Takes key presses and adjusts the dot's velocity
	void handleEvent(SDL_Event& e);

	//Moves the dot
	void move();

	//Shows the dot on the screen
	void render();

	//The rectangular colliders for the player
	SDL_Rect pColliderLeft;
	SDL_Rect pColliderMid;
	SDL_Rect pColliderRight;

// FOR NOW private:
	//The X and Y offsets of the dot
	int mPosX, mPosY;

	//The velocity of the dot
	int mVelX, mVelY;
};

class Enemy
{
public:
	//LENGTH and Width of the enemy
	static const int ENEMY_WIDTH = 50;
	static const int ENEMY_HEIGHT = 25;

	int healthPoints = 1;
	bool isAlive = true;
	//The direction of the enemy
	bool dir = true;

	//moving the enemy right
	void move();

	//Render the enemy on the screen, as long
	// as the enemy is alive-the boolean function
	bool render(SDL_Rect& playerBullet, Player& playerObj, int& ballXDir, int& ballYDir, SDL_Rect& ballRect);

	Enemy(int posX, int posY);

	int ePosX, ePosY, eVel;

	//The rectangular colliders for the enemy
	SDL_Rect eRect;
	SDL_Rect eColliderUp;
	SDL_Rect eColliderDown;
	SDL_Rect eColliderRight;
	SDL_Rect eColliderLeft;

};

//This is the window that will be rendered
SDL_Window* gWindow = NULL;

//This is the surface that is wthin the Window
SDL_Surface* screenSurface = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//texter of the player
LTexture gPlayerTexture;

SDL_Joystick* gGameController = NULL;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface != NULL)
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	else
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}


	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

Player::Player()
{
	//Initialize the offsets
	mPosX = ( SCREEN_WIDTH / 2 ) - ( PLAYER_WIDTH / 2);
	mPosY = SCREEN_HEIGHT - PLAYER_HEIGHT - 20;

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;

	healthPoints = 1;
	lives = 3;
	score = 0;

	pColliderLeft.w = 26;
	pColliderLeft.h = 26;
	pColliderLeft.x = mPosX;
	pColliderLeft.y = mPosY;

	pColliderMid.w = 26;
	pColliderMid.h = 26;
	pColliderMid.x = mPosX + 26 ;
	pColliderMid.y = mPosY;

	pColliderRight.w = 26;
	pColliderRight.h = 26;
	pColliderRight.x = mPosX + 52;
	pColliderRight.y = mPosY;



}

void Player::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY -= PLAYER_VEL; break;
		case SDLK_DOWN: mVelY += PLAYER_VEL; break;
		case SDLK_LEFT: mVelX -= PLAYER_VEL; break;
		case SDLK_RIGHT: mVelX += PLAYER_VEL; break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY += PLAYER_VEL; break;
		case SDLK_DOWN: mVelY -= PLAYER_VEL; break;
		case SDLK_LEFT: mVelX += PLAYER_VEL; break;
		case SDLK_RIGHT: mVelX -= PLAYER_VEL; break;
		}
	}
}

void Player::move()
{
	pColliderLeft.x = mPosX;
	pColliderMid.x = mPosX + 26;
	pColliderRight.x = mPosX + 52;

	//Move the dot left or right
	mPosX += mVelX;

	//If the dot went too far to the left or right
	if ((mPosX < 0) || (mPosX + PLAYER_WIDTH > SCREEN_WIDTH))
	{
		//Move back
		mPosX -= mVelX;
	}
}

void Player::render()
{
	//display the player on the screen
	gPlayerTexture.render(mPosX, mPosY);
}

Enemy::Enemy(int posX, int posY)
{
	ePosX = posX;
	ePosY = posY;

	eRect.w = ENEMY_WIDTH;
	eRect.h = ENEMY_HEIGHT;
	eRect.x = ePosX;
	eRect.y = ePosY;
	eColliderUp.w = ENEMY_WIDTH - 4;
	eColliderUp.h = ENEMY_HEIGHT / 2;
	eColliderUp.x = ePosX + 2;
	eColliderUp.y = ePosY;

	eColliderDown.w = ENEMY_WIDTH - 4;
	eColliderDown.h = ENEMY_HEIGHT / 2;
	eColliderDown.x = ePosX + 2;
	eColliderDown.y = ePosY + ENEMY_HEIGHT / 2;

	eColliderRight.w = ENEMY_WIDTH / 2;
	eColliderRight.h = ENEMY_HEIGHT - 4;
	eColliderRight.x = ePosX + ENEMY_WIDTH / 2;
	eColliderRight.y = ePosY + 2;

	eColliderLeft.w = ENEMY_WIDTH / 2 + 2;
	eColliderLeft.h = ENEMY_HEIGHT - 4;
	eColliderLeft.x = ePosX;
	eColliderLeft.y = ePosY;
	eVel = 4;
}

//TODO use the SDL Collision function for the collision detection, dont be a hipster and make your own
bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	//If none of the sides from A are outside B
	return true;
}

void Enemy::move()
{
	if (isAlive)
	{
	}
	else
	{
		eColliderUp.y = 0 - ENEMY_HEIGHT;
		eColliderUp.x = 0 - ENEMY_WIDTH;
		eColliderDown.y = 0 - ENEMY_HEIGHT;
		eColliderDown.x = 0 - ENEMY_WIDTH;
		eColliderLeft.y = 0 - ENEMY_HEIGHT;
		eColliderLeft.x = 0 - ENEMY_WIDTH;
		eColliderRight.y = 0 - ENEMY_HEIGHT;
		eColliderRight.x = 0 - ENEMY_WIDTH;


	}
}

bool Enemy::render(	SDL_Rect& playerBullet, Player& playerObj, int& ballXDir,
					int& ballYDir, SDL_Rect& ballRect)
{
	if (checkCollision(ballRect, eColliderUp) && ballYDir == +3)
	{
		ballYDir = -3;
		if (ballXDir == -3)
			ballXDir == +3;
		if (ballXDir == +3)
			ballXDir == -3;
		healthPoints--;
	}
	if (checkCollision(ballRect, eColliderDown) && ballYDir == -3)
	{
		ballYDir = +3;
		if (ballXDir == -3)
			ballXDir == +3;
		if (ballXDir == +3)
			ballXDir == -3;
		healthPoints--;
		isAlive = false;
	}
	if (checkCollision(ballRect, eColliderLeft) && ballYDir == -3)
	{
		ballXDir = -3;
		healthPoints--;
		isAlive = false;
	}
	if (checkCollision(ballRect, eColliderRight) && ballYDir == -3)
	{
		ballXDir = +3;
		healthPoints--;
		isAlive = false;
	}
	if (checkCollision(eColliderDown, playerBullet))
	{
		healthPoints = 0;

		playerObj.score += 100;

		return false;
	}
	else
	  {
		if (healthPoints == 1)
			isAlive = true;
		else if (healthPoints == 0)
			isAlive = false;
		if (isAlive)
		{

			SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0xFF, 0xFF);
			SDL_RenderFillRect(gRenderer, &eRect);

			/* To see the collision of the box
			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
			SDL_RenderDrawRect(gRenderer, &eColliderUp);

			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
			SDL_RenderDrawRect(gRenderer, &eColliderDown);

			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
			SDL_RenderDrawRect(gRenderer, &eColliderRight);

			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
			SDL_RenderDrawRect(gRenderer, &eColliderLeft);
			*/
			return true;
		}
		else if (!isAlive)
			return false;
		return false;
	}
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL Subsystems 
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}
		//Make sure there is a joystick
		if (SDL_NumJoysticks() < 1)
			printf("No Joystick");
		else
		{
			//Load the joystick
			gGameController = SDL_JoystickOpen(0);
			if (gGameController == NULL)
				printf("Warning: unable to use the game controller SDL Error: %s\n", SDL_GetError() );
		}
		//Create window
		gWindow = SDL_CreateWindow("Brick Breaker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load dot texture
	if (!gPlayerTexture.loadFromFile("Player.bmp"))
	{
		printf("Failed to load dot texture!\n");
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded images
	gPlayerTexture.free();

	//Close the game controller if there is one
	if (SDL_NumJoysticks() > 1)
	{
		SDL_JoystickClose(gGameController);
		gGameController = NULL;
	}

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{

			//Event handler
			SDL_Event e;

			/*Game Controller Continue
			//Normalized direction
			int xDir = 0;
			int yDir = 0;
			*/
			//LTimer fpsTimer;

			//Current time start time
			//Uint32 startTime = 0;

			//In memory text stream
			//std::stringstream timeText;

			//The Player that will be moving around on the screen
			Player player;

			Enemy darthVader(100, 80);
			Enemy stormTrooper(165, 80);
			Enemy bobaFett(230, 80);
			Enemy lukeSkywalker(295, 80);
			Enemy yodaYoda(360, 80);
			Enemy obiWan(425, 80);
			Enemy hanSolo(490, 80);
			Enemy deathStar(555, 80);
			Enemy darthVader2(100, 120);
			Enemy stormTrooper2(165, 120);
			Enemy bobaFett2(230, 120);
			Enemy lukeSkywalker2(295, 120);
			Enemy yodaYoda2(360, 120);
			Enemy obiWan2(425, 120);
			Enemy hanSolo2(490, 120);
			Enemy deathStar2(555, 120);
			Enemy darthVader3(100, 160);
			Enemy stormTrooper3(165, 160);
			Enemy bobaFett3(230, 160);
			Enemy lukeSkywalker3(295, 160);
			Enemy yodaYoda3(360, 160);
			Enemy obiWan3(425, 160);
			Enemy hanSolo3(490, 160);
			Enemy deathStar3(555, 160);

			int ballX = SCREEN_WIDTH / 2;
			int ballY = SCREEN_HEIGHT / 2;
			int ballYDir = 0;
			int ballXDir = 0;

			//While application is running
			while (isRunning)
			{

				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						isRunning = false;
					}
					//Reset start time on return keypress
					else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
					{
						//startTime = SDL_GetTicks();
					}
					//Handle input for the dot
					player.handleEvent(e);
				}

				//Move the Player
				player.move();

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderClear(gRenderer);

				SDL_Rect ballRect = { ballX, ballY, 20, 20 };
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderFillRect(gRenderer, &ballRect);

				darthVader.render(ballRect, player, ballXDir, ballYDir, ballRect);
				darthVader.move();
				stormTrooper.render(ballRect, player, ballXDir, ballYDir, ballRect);
				stormTrooper.move();
				bobaFett.render(ballRect, player, ballXDir, ballYDir, ballRect);
				bobaFett.move();
				lukeSkywalker.render(ballRect, player, ballXDir, ballYDir, ballRect);
				lukeSkywalker.move();
				yodaYoda.render(ballRect, player, ballXDir, ballYDir, ballRect);
				yodaYoda.move();
				obiWan.render(ballRect, player, ballXDir, ballYDir, ballRect);
				obiWan.move();
				hanSolo.render(ballRect, player, ballXDir, ballYDir, ballRect);
				hanSolo.move();
				deathStar.render(ballRect, player, ballXDir, ballYDir, ballRect);
				deathStar.move();

				darthVader2.render(ballRect, player, ballXDir, ballYDir, ballRect);
				darthVader2.move();
				stormTrooper2.render(ballRect, player, ballXDir, ballYDir, ballRect);
				stormTrooper2.move();
				bobaFett2.render(ballRect, player, ballXDir, ballYDir, ballRect);
				bobaFett2.move();
				lukeSkywalker2.render(ballRect, player, ballXDir, ballYDir, ballRect);
				lukeSkywalker2.move();
				yodaYoda2.render(ballRect, player, ballXDir, ballYDir, ballRect);
				yodaYoda2.move();
				obiWan2.render(ballRect, player, ballXDir, ballYDir, ballRect);
				obiWan2.move();
				hanSolo2.render(ballRect, player, ballXDir, ballYDir, ballRect);
				hanSolo2.move();
				deathStar2.render(ballRect, player, ballXDir, ballYDir, ballRect);
				deathStar2.move();

				darthVader3.render(ballRect, player, ballXDir, ballYDir, ballRect);
				darthVader3.move();
				stormTrooper3.render(ballRect, player, ballXDir, ballYDir, ballRect);
				stormTrooper3.move();
				bobaFett3.render(ballRect, player, ballXDir, ballYDir, ballRect);
				bobaFett3.move();
				lukeSkywalker3.render(ballRect, player, ballXDir, ballYDir, ballRect);
				lukeSkywalker3.move();
				yodaYoda3.render(ballRect, player, ballXDir, ballYDir, ballRect);
				yodaYoda3.move();
				obiWan3.render(ballRect, player, ballXDir, ballYDir, ballRect);
				obiWan3.move();
				hanSolo3.render(ballRect, player, ballXDir, ballYDir, ballRect);
				hanSolo3.move();
				deathStar3.render(ballRect, player, ballXDir, ballYDir, ballRect);
				deathStar3.move();

				//Render objects
				player.render();

				ballX += ballXDir;
				ballY += ballYDir;

				if (ballX <= 0)
					ballXDir = +3;
				if (ballX >= 780)
					ballXDir = -3;
				if (ballY <= 0)
					ballYDir = +3;
				if (ballY >= 580)
				{
					ballYDir = -3;
					isRunning = false;
				}
				if (checkCollision(ballRect, player.pColliderMid) && ballYDir == +3)
				{
					ballYDir = -3;
				}
				if (checkCollision(ballRect, player.pColliderLeft) && ballYDir == +3)
				{
					ballYDir = -3;
					if (ballXDir == +3)
						ballXDir = -3;
				}
				if (checkCollision(ballRect, player.pColliderRight) && ballYDir == +3)
				{
					ballYDir = -3;
					if (ballXDir == -3)
						ballXDir = +3;
				}
				//std::cout << "Score is: " << player.score << std::endl;
				//std::cout << "Milliseconds since start time " << SDL_GetTicks() - startTime << std::endl;
				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	std::cout << "Closing down the window! T-2sec" << std::endl;
	

	SDL_Delay(2000);

	//Free the resources and close SDL
	close();

	return 0;
}
