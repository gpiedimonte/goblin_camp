/* Copyright 2011 Ilkka Halila
This file is part of Goblin Camp.

Goblin Camp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Goblin Camp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License 
along with Goblin Camp. If not, see <http://www.gnu.org/licenses/>.*/
#include "stdafx.hpp"

#include "tileRenderer/sdl/SDLTilesetRenderer.hpp"
#include "tileRenderer/sdl/SDLSprite.hpp"

#include "Logger.hpp"
#include "data/Config.hpp"
#include "MathEx.hpp"


SDLTilesetRenderer::SDLTilesetRenderer(int screenWidth, int screenHeight, TCODConsole * mapConsole)
: TilesetRenderer(screenWidth, screenHeight, mapConsole),
  translucentUI(Config::GetCVar<bool>("translucentUI")),
  mapSurface()
{
	TCODSystem::registerSDLRenderer(this, translucentUI);
	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	SDL_Surface * temp = SDL_CreateRGBSurface(0, MathEx::NextPowerOfTwo(screenWidth), MathEx::NextPowerOfTwo(screenHeight), 32, rmask, gmask, bmask, amask);
	SDL_SetAlpha(temp, 0, SDL_ALPHA_OPAQUE);
	mapSurface = boost::shared_ptr<SDL_Surface>(SDL_DisplayFormat(temp), SDL_FreeSurface);
	SDL_FreeSurface(temp);

	if (!mapSurface)
	{
		LOG(SDL_GetError());
	}
}

SDLTilesetRenderer::~SDLTilesetRenderer() {
}

Sprite_ptr SDLTilesetRenderer::CreateSprite(SpriteLayerType spriteLayer, boost::shared_ptr<TileSetTexture> tilesetTexture, int tile) {
	return Sprite_ptr(new SDLSprite(this, tilesetTexture, tile));
}

Sprite_ptr SDLTilesetRenderer::CreateSprite(SpriteLayerType spriteLayer, boost::shared_ptr<TileSetTexture> tilesetTexture, const std::vector<int>& tiles, bool connectionMap, int frameRate, int frameCount) {
	return Sprite_ptr(new SDLSprite(this, tilesetTexture, tiles.begin(), tiles.end(), connectionMap, frameRate, frameCount));
}

void SDLTilesetRenderer::PreDrawMap() {
	SDL_Rect viewportRect;
	viewportRect.x = startPixelX;
	viewportRect.y = startPixelY;
	viewportRect.w = pixelW;
	viewportRect.h = pixelH;
	SDL_SetClipRect(mapSurface.get(), &viewportRect);
}

void SDLTilesetRenderer::PostDrawMap() {
	SDL_SetClipRect(mapSurface.get(), 0);
}
	
void SDLTilesetRenderer::DrawSprite(int screenX, int screenY, boost::shared_ptr<TileSetTexture> texture, int tile) const {
	texture->DrawTile(tile, mapSurface.get(), &CalcDest(screenX, screenY));
}

void SDLTilesetRenderer::DrawSpriteCorner(int screenX, int screenY, boost::shared_ptr<TileSetTexture> texture, int tile, Corner corner) const {
	texture->DrawTileCorner(tile, corner, mapSurface.get(), &CalcDest(screenX, screenY));
}


void SDLTilesetRenderer::DrawNullTile(int screenX, int screenY) {
	SDL_FillRect(mapSurface.get(), &CalcDest(screenX, screenY), 0);
}

namespace {
	inline void setPixelAlpha(SDL_Surface *surface, int x, int y, Uint32 keyColor)
	{
		SDL_PixelFormat *fmt = surface->format;
		int bpp = fmt->BytesPerPixel;
		if (bpp != 4) return;

		Uint32 *p = (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch) + x;
		Uint32 c = (*p | fmt->Amask);
		if (c == keyColor) {
			*p = *p & ~fmt->Amask;
		} else if (c == fmt->Amask) {
			*p = (*p & ~fmt->Amask) | (128 << fmt->Ashift);
		}
	}
}

void SDLTilesetRenderer::render(void *surf, void*sdl_screen) {
	SDL_Surface *tcod = (SDL_Surface *)surf;
	SDL_Surface *screen = (SDL_Surface *)sdl_screen;

	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();
	TCODColor keyColor = GetKeyColor();

	SDL_Rect srcRect={0,0,screenWidth,screenHeight};
	SDL_Rect dstRect={0,0,screenWidth,screenHeight};
	
	if (translucentUI) {
		Uint32 keyColorVal = SDL_MapRGBA(tcod->format, keyColor.r, keyColor.g, keyColor.b, 255);
		if (SDL_MUSTLOCK(tcod))
		{
			SDL_LockSurface(tcod);
		}
		for (int x = 0; x < screenWidth; ++x) {
			for (int y = 0; y < screenHeight; ++y) {
				setPixelAlpha(tcod,x,y, keyColorVal);
			}
		}
		if (SDL_MUSTLOCK(tcod))
		{
			SDL_UnlockSurface(tcod);
		}
	}
	else {
		SDL_SetColorKey(tcod,SDL_SRCCOLORKEY, SDL_MapRGBA(tcod->format, keyColor.r, keyColor.g, keyColor.b, 255));
	}
	SDL_LowerBlit(tcod, &srcRect, mapSurface.get(), &dstRect);
	SDL_LowerBlit(mapSurface.get(), &srcRect, screen, &dstRect);
}