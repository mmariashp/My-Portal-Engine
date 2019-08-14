#include "main_head.h"
#define EyeHeight  5
#define DuckHeight 2.5
#define HeadMargin 1
#define KneeHeight 2

#define Yaw(y,z) (y + z*player.yaw)

float scaleH = 34;

void			draw_yellow_line(SDL_Renderer *ren, t_vector start, t_vector end)
{
	SDL_SetRenderDrawColor(ren, 0xff, 0xff, 0, 200);
	SDL_RenderDrawLine(ren, start.x, start.y, end.x, end.y);
	SDL_SetRenderDrawColor(ren, 0xff, 0xff, 0xff, 0xff);
}

void			draw_line_any_colar(SDL_Renderer *ren,  t_vector start, t_vector end, int r, int g, int b)
{
	SDL_SetRenderDrawColor(ren, r, g, b, 200);
	SDL_RenderDrawLine(ren, start.x, start.y, end.x, end.y);
	SDL_SetRenderDrawColor(ren, 0xff, 0xff, 0xff, 0xff);
}

static void vline(SDL_Surface *surface, int x, int y1, int y2, int top, int middle, int bottom)
{
    int *pix = (int*) surface->pixels;
    y1 = clamp(y1, 0, H-1);
    y2 = clamp(y2, 0, H-1);
    if(y2 == y1)
        pix[y1*W+x] = middle;
    else if(y2 > y1)
    {
        pix[y1*W+x] = top;
        for(int y=y1+1; y<y2; ++y) pix[y*W+x] = middle;
        pix[y2*W+x] = bottom;
    }
}



float				len_between_points(t_vector a, t_vector b)
{
	return (sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y)));
}

float				get_triangle_height(t_vector a, t_vector b, t_vector c)
{
	int				ab;
	int				bc;
	int				ca;
	int				p;

	ab = len_between_points(a, b);
	bc = len_between_points(b, c);
	ca = len_between_points(c, a);
	p = (ab + bc + ca) >> 1;
	return (2 * (sqrt (p * (p - ab) * (p - bc) * (p - ca))) / ab);
}

int				get_nearest_line_to_dot(t_vector a, t_vector d1, t_vector d2)
{
	int 		ad1;
	int			ad2;

	ad1 = len_between_points(a, d1);
	ad2 = len_between_points(a, d2);
	return min(ad1, ad2);
}

void			sort_by_nearest(t_wall **walls, t_player player, signed short *tab, unsigned short max)
{
	int			i;
	int			j;
	float 		i_dist;
	float 		j_dist;
	int			tmp;
	
	i = 0;
	while (i < max && tab[i] != -1)
	{
		j = 0;
		i_dist = get_nearest_line_to_dot(player.pos, walls[tab[i]]->start, walls[tab[i]]->end);
		while (j < max && tab[j] != -1)
		{
			if (tab[i] != tab[j])
			{
				j_dist = get_nearest_line_to_dot(player.pos, walls[tab[j]]->start, walls[tab[j]]->end);
				if (i_dist < j_dist)
				{
					tmp = tab[i];
					tab[i] = tab[j];
					tab[j] = tmp;
				}
			}
			j++;
		}
		i++;
	}
}


void 			make_intersect(t_wall *wall)
{
	t_vector 	i1;
	t_vector 	i2;
	float nearz = 1e-4f, farz = 5, nearside = 1e-5f, farside = 1000.f;

	i1 = Intersect(wall->start.x, wall->start.y, wall->end.x, wall->end.y, -nearside,nearz, -farside,farz);
    i2 = Intersect(wall->start.x, wall->start.y, wall->end.x, wall->end.y,  nearside,nearz,  farside,farz);
	if(wall->start.y < nearz) 
	{
		if(i1.y > 0)
		{
			wall->start.x = i1.x;
			wall->start.y = i1.y;
		}
		else
		{
			wall->start.x = i2.x;
			wall->start.y = i2.y;
		}
	}
    if(wall->end.y < nearz)
	{
		if(i1.y > 0)
		{
			wall->end.x = i1.x;
			wall->end.y = i1.y;
		}
		else
		{
			wall->end.x = i2.x;
			wall->end.y = i2.y;
		}
	}
}

void			draw_sectors(t_sector *sectors, t_player player, t_sdl *sdl, t_draw_data *data);

int t = 1;


int			calc_floor_ceil(t_player player, float floor_or_ceil, float scale_y)
{
	return (player.half_win_size.y - (floor_or_ceil - player.height) * scale_y);
}

void 			draw_world(t_sector *sec, t_wall wall, t_player player, t_sdl *sdl, t_draw_data data)
{
	t_vector	line_start;
	t_vector	line_end;
	t_vector 	scale1;
	t_vector 	scale2;
	t_wall		cp;
	int 		ceil_y_s;
	int 		ceil_y_e;
	int 		floor_y_s;
	int 		floor_y_e;
	int 		n_ceil_y_s;
	int 		n_ceil_y_e;
	int 		n_floor_y_s;
	int 		n_floor_y_e;
	int 		cya;
	int 		cyb;
	int 		ya; 
	int 		yb;
	int 		n_cya;
	int 		n_cyb;
	int			x;
	int 		end;
	int 		u0, u1;

	cp = wall;

	line_start = (t_vector){wall.start.x - player.pos.x, wall.start.y - player.pos.y, wall.start.z - player.pos.z};
	line_end = (t_vector){wall.end.x - player.pos.x, wall.end.y - player.pos.y, wall.end.z - player.pos.z};	


	wall.start = (t_vector){line_start.x * player.sin_angl - line_start.y * player.cos_angl,
				line_start.x * player.cos_angl + line_start.y * player.sin_angl, .z = wall.start.y};
	wall.end = (t_vector){line_end.x * player.sin_angl - line_end.y * player.cos_angl,
			line_end.x * player.cos_angl + line_end.y * player.sin_angl, .z = wall.end.y};
	if (wall.start.y <= 0 && wall.end.y <= 0)
		return ;

	int scaleL;
    if(fabsf(cp.start.x - cp.end.x) > fabsf(cp.start.y - cp.end.y))
        scaleL = fabsf(cp.start.x - cp.end.x) / 5.0f;
    else
        scaleL = fabsf(cp.start.y - cp.end.y) / 5.0f;

	if(wall.type != empty_wall)
    	u0 = 0, u1 = wall.texture->w * scaleL - 1;
	
	t_vector org1 = {wall.start.x, wall.start.y}, org2 = {wall.end.x, wall.end.y};
	
	if (wall.start.y <= 0 || wall.end.y <= 0)
		make_intersect(&wall);
	if (wall.type != empty_wall)
	{
		if(fabs(wall.end.x - wall.start.x) > fabs(wall.end.y-wall.start.y))
            u0 = (wall.start.x-org1.x) * (wall.texture->w * scaleL- 1) / (org2.x-org1.x), u1 = (wall.end.x-org1.x) * (wall.texture->w * scaleL - 1) / (org2.x-org1.x);
    	else
    	    u0 = (wall.start.y-org1.y) * (wall.texture->w * scaleL- 1) / (org2.y-org1.y), u1 = (wall.end.y-org1.y) * (wall.texture->w * scaleL - 1) / (org2.y-org1.y);
	
	}
	scale1 =(t_vector) {player.hfov / wall.start.y, player.vfov / wall.start.y};
    scale2 =(t_vector) {player.hfov / wall.end.y, player.vfov / wall.end.y};

	wall.start.x = player.half_win_size.x - (int)(wall.start.x * scale1.x);
	wall.end.x = player.half_win_size.x - (int)(wall.end.x * scale2.x);

	if(wall.start.x >= wall.end.x || wall.end.x < data.start || wall.start.x > data.end)
		return ;

	ceil_y_s = calc_floor_ceil(player, sec->ceil, scale1.y);
	ceil_y_e = calc_floor_ceil(player, sec->ceil, scale2.y);
	floor_y_s = calc_floor_ceil(player, sec->floor, scale1.y);
	floor_y_e = calc_floor_ceil(player, sec->floor, scale2.y);
	
	if(wall.type == empty_wall)
	{
		if (!wall.sectors[1])
		{
			printf("wall.sectors[1] is empty. Wall #%d, sector #%d\n", wall.id, wall.sectors[0]->sector);
			return;
		}
		n_ceil_y_s = calc_floor_ceil(player, min(wall.sectors[0]->ceil, wall.sectors[1]->ceil), scale1.y);
		n_ceil_y_e = calc_floor_ceil(player, min(wall.sectors[0]->ceil, wall.sectors[1]->ceil), scale2.y);
    	n_floor_y_s = calc_floor_ceil(player, max(wall.sectors[0]->floor, wall.sectors[1]->floor), scale1.y);
		n_floor_y_e = calc_floor_ceil(player, max(wall.sectors[0]->floor, wall.sectors[1]->floor), scale2.y);
	}

	x = max(wall.start.x, data.start);
	end = min(wall.end.x, data.end);

	data.start = x;
	data.end = end;
	while (x < end && x < sdl->win_size.x)
	{
		int txtx = (u0 * ((wall.end.x - x) * wall.end.y) + u1 * ((x - wall.start.x) * wall.start.y)) / ((wall.end.x - x) * wall.end.y + (x - wall.start.x) * wall.start.y);
		
		ya = (x - wall.start.x) * (ceil_y_e - ceil_y_s) / (wall.end.x-wall.start.x) + ceil_y_s;

		cya = clamp(ya, data.ytop[x], data.ybottom[x]);

        yb = (x - wall.start.x) * (floor_y_e - floor_y_s) / (wall.end.x-wall.start.x) + floor_y_s;

		cyb = clamp(yb, data.ytop[x], data.ybottom[x]);

		
		
	//	SDL_SetRenderDrawColor(sdl->ren, 102, 100, 98, 255);
	//	SDL_RenderDrawLine(sdl->ren, x, data.ytop[x] , x, cya - 1);

		vline(sdl->surf, x, data.ytop[x], cya - 1, 102, 100, 98);

	//	SDL_SetRenderDrawColor(sdl->ren, 200, 200, 200, 255);
	//	SDL_RenderDrawPoint(sdl->ren, x, cya - 1);

	//	SDL_SetRenderDrawColor(sdl->ren, 73, 52, 0, 255);
	//	SDL_RenderDrawLine(sdl->ren, x, cyb, x, data.ybottom[x]);

		vline(sdl->surf, x, cyb, data.ybottom[x], 73, 52, 0);
		if (wall.type != empty_wall)
		{
		//	SDL_SetRenderDrawColor(sdl->ren, 160, 130, 65, 255);
		//	SDL_RenderDrawLine(sdl->ren, x, cya, x, cyb);
			//if (t % 2 == 0)
		//	textLine(x, cya, cyb, (struct Scaler)Scaler_Init(ya, cya, yb, 0, wall.texture->h), txtx, sdl->surf, wall.texture);
			textLine(x, cya, cyb, (struct Scaler)Scaler_Init(ya,cya,yb, 0, fabsf(sec->floor - sec->ceil) * scaleH), txtx, sdl->surf, wall.texture);
			data.ybottom[x] = cyb;
			data.ytop[x] = cya;
		}
		else
		{
			n_cya = clamp((x - wall.start.x) * (n_ceil_y_e - n_ceil_y_s) / (wall.end.x-wall.start.x) + n_ceil_y_s,
			data.ytop[x], data.ybottom[x]);
			n_cyb = clamp((x - wall.start.x) * (n_floor_y_e - n_floor_y_s) / (wall.end.x-wall.start.x) + 
			n_floor_y_s, data.ytop[x], data.ybottom[x]);

	//	SDL_SetRenderDrawColor(sdl->ren, 0, 0, 98, 255);
	//	SDL_RenderDrawLine(sdl->ren, x, cya, x, n_cya);

		vline(sdl->surf, x, cya, n_cya, 255, 255, 200);

	//	SDL_SetRenderDrawColor(sdl->ren, 200, 200, 200, 255);
	//	SDL_RenderDrawPoint(sdl->ren, x, n_cya - 1);

	//	SDL_SetRenderDrawColor(sdl->ren, 22, 0, 118, 255);
	//	SDL_RenderDrawLine(sdl->ren, x, n_cyb, x, cyb);

		vline(sdl->surf, x, n_cyb, cyb, 255, 200, 0);

	//	SDL_SetRenderDrawColor(sdl->ren, 200, 200, 200, 255);
	//	SDL_RenderDrawPoint(sdl->ren, x, n_cyb + 1);
			

			data.ytop[x] = n_cya;
			data.ybottom[x] = n_cyb;
		}
	//	SDL_SetRenderDrawColor(sdl->ren, 200, 200, 200, 255);
	//	SDL_RenderDrawPoint(sdl->ren, x, cyb + 1);
		x++;
	}
//	SDL_SetRenderDrawColor(sdl->ren, 200, 200, 200, 255);
//	SDL_RenderDrawLine(sdl->ren, x - 1, cya, x - 1, cyb);
	if (wall.type == empty_wall)
	{
		if (wall.sectors[0]->sector != player.curr_sector->sector && wall.sectors[0]->sector != sec->sector)
			draw_sectors(wall.sectors[0], player, sdl, &data);
		else if (wall.sectors[1]->sector != player.curr_sector->sector && wall.sectors[1]->sector != sec->sector)
			draw_sectors(wall.sectors[1], player, sdl, &data);
	}
}

void			draw_sectors(t_sector *sec, t_player player, t_sdl *sdl, t_draw_data *data)
{
	int			i;
	int			p;
	int			wall;

	i = 0;
	p = 0;
	while (i < sec->n_walls)
	{
		if(sec->wall[i]->type != empty_wall)
			draw_world(sec, *sec->wall[i], player, sdl, *data);
		i++;
	}
//	sort_by_nearest(sec->wall, player, sec->portals, MAX_PORTALS);
	while (p < MAX_PORTALS && sec->portals[p] >= 0)
	{
		draw_world(sec, *sec->wall[sec->portals[p]], player, sdl, *data);
		p++;
	}
}

void				run_with_buff(t_player player, t_sdl *sdl, unsigned int win_x)
{
	unsigned 		x; 
	int 			ytop[win_x];
	int				ybottom[win_x];
	t_draw_data		draw_data;

	x = 0;
    while(x < win_x)
	{
		ybottom[x] = sdl->win_size.y - 1;
		ytop[x] = 0;
		x++;
	}
	draw_data.start = 0;
	draw_data.end = win_x;
	draw_data.ytop = &ytop[0];
	draw_data.ybottom = &ybottom[0];
	draw_sectors(player.curr_sector, player, sdl, &draw_data);
}

void			move_player(t_player *player, float sin_angle, float cos_angle)
{
	int			i;

	i = 0;
	while (i < player->curr_sector->n_walls)
	{
		if(IntersectBox(player->pos.x,player->pos.y, player->pos.x + cos_angle, player->pos.y + sin_angle,
			player->curr_sector->wall[i]->start.x, player->curr_sector->wall[i]->start.y,
			player->curr_sector->wall[i]->end.x, player->curr_sector->wall[i]->end.y)
        && PointSide( player->pos.x + cos_angle, player->pos.y + sin_angle,player->curr_sector->wall[i]->start.x, player->curr_sector->wall[i]->start.y,
			player->curr_sector->wall[i]->end.x, player->curr_sector->wall[i]->end.y) < 0)
        {
			if (player->curr_sector->wall[i]->type == fieled_wall)
				return;
			if (player->curr_sector->wall[i]->sectors[0] &&
				player->curr_sector->sector != player->curr_sector->wall[i]->sectors[0]->sector) 
				player->curr_sector = player->curr_sector->wall[i]->sectors[0];
			else if (player->curr_sector->wall[i]->sectors[1] &&
				player->curr_sector->sector != player->curr_sector->wall[i]->sectors[1]->sector) 
				player->curr_sector = player->curr_sector->wall[i]->sectors[1];
			player->pos = (t_vector){player->pos.x + cos_angle, player->pos.y + sin_angle};
			//player->height = EyeHeight + (player->curr_sector->floor > 0 ? player->curr_sector->floor - 1 : player->curr_sector->floor);
			player->height = EyeHeight + player->curr_sector->floor;
			break;
        }
		i++;
	}
	player->pos = (t_vector){player->pos.x + cos_angle, player->pos.y + sin_angle};
	sort_by_nearest(player->curr_sector->wall, *player, player->curr_sector->portals, MAX_PORTALS);
}

int 				is_it_wall(t_vector pos, t_wall wall)
{
	t_point			n;
	int 			p;

	n = (t_point){wall.start.y - wall.end.y, wall.end.x - wall.start.x};
	p = (n.x * (pos.x - wall.start.x) + n.y * (pos.y - wall.start.y)) / sqrtf(n.x * n.x + n.y * n.y);
	return (p);
}
/*
void				move_player(t_player *player, float sin_angle, float cos_angle)
{
	int				i;
	int				intersect;
	t_wall			*wall;
	t_sector		*curr_sec;

	i = 0;
	curr_sec = player->curr_sector;
	printf("\n\tPlayer in sectot %d pos: %f, %f\n", player->curr_sector->sector, player->pos.x, player->pos.y);
	while (i < curr_sec->n_walls)
	{
		wall = curr_sec->wall[i];
		if (wall->type != empty_wall)
		{
			printf("Wall #%d (%f, %f; %f, %f)\n", wall->id, wall->start.x, wall->start.y, wall->end.x, wall->end.y);
			intersect = is_it_wall((t_vector){player->pos.x + cos_angle, player->pos.y + sin_angle}, *wall);
			printf("intersect %d\n\n",intersect);
			if (intersect < 2)
				return ;
		}
		i++;
	}
	i = 0;
	while (i < MAX_PORTALS && curr_sec->portals[i] > -1)
	{
		wall = curr_sec->wall[curr_sec->portals[i]];printf("Wall #%d (%f, %f; %f, %f)\n", wall->id, wall->start.x, wall->start.y, wall->end.x, wall->end.y);
		intersect = is_it_wall((t_vector){player->pos.x + cos_angle, player->pos.y + sin_angle}, *wall);
		printf("intersect %d\n\n",intersect);
		if (intersect < 1)
		{
			if (wall->sectors[0]->sector != player->curr_sector->sector)
				player->curr_sector = wall->sectors[0];
			else if (wall->sectors[1]->sector != player->curr_sector->sector)
				 player->curr_sector = wall->sectors[1];				
			player->pos = (t_vector){player->pos.x + cos_angle, player->pos.y + sin_angle};
			break ;
		}
		i++;
	}
	player->pos = (t_vector){player->pos.x + cos_angle, player->pos.y + sin_angle};
}
*/
int					hook_event(t_player *player)
{
	SDL_Event		e;

	while(SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			return (0);
		else if (e.type == SDL_KEYDOWN)
		{
			if (e.key.keysym.sym == SDLK_w || e.key.keysym.sym == SDLK_UP)
				move_player(player, player->sin_angl, player->cos_angl);
			if (e.key.keysym.sym == SDLK_s || e.key.keysym.sym == SDLK_DOWN)
				move_player(player, -player->sin_angl, -player->cos_angl);
			if (e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_RIGHT)
			{
				if (e.key.keysym.sym == SDLK_LEFT)
					player->angle -= 0.05;	
				else
					player->angle += 0.05;
				player->cos_angl = cos(player->angle);
				player->sin_angl = sin(player->angle);
			}
			if (e.key.keysym.sym == 'p')
			{
				scaleH++;
			//	t++;
			}
			if (e.key.keysym.sym == 'm')
				scaleH--;
			if (e.key.keysym.sym == SDLK_a)
				move_player(player, -player->cos_angl, player->sin_angl);
			if (e.key.keysym.sym == SDLK_d)
				move_player(player, player->cos_angl, -player->sin_angl);
			if (e.key.keysym.sym == SDLK_ESCAPE)
				return (0);
		}
	}
	return (1);
}

void			game_loop(t_sdl *sdl, t_player player, t_sector *sectors)
{
	int			run;
	SDL_Texture *tex;
	t_timer				timer;
	char				str[100];
	SDL_Texture			*text;

	player.cos_angl = cos(player.angle);
	player.sin_angl = sin(player.angle);
	run = 1;
	sdl->frame_id = 0;
    timer = init_timer();
    start_timer(&timer);
	SDL_Surface *textSurface;
	while(run)
	{
		if((sdl->fps = (float)sdl->frame_id / (get_ticks(timer) / 1000.f)) > 2000000)
            sdl->fps = 0;
		sdl->frame_id++;
		sprintf(str, "fps:  %f", sdl->fps);
		printf("got fps = %f\n", sdl->fps);
		text = make_color_text(sdl->ren, str, "VCR_OSD_MONO_1.001.ttf", 100, (SDL_Color){.a = 250, .r = 255, .g = 255, .b = 255});
		
		SDL_SetRenderDrawColor(sdl->ren, 0, 0, 0, 255);
		SDL_RenderClear(sdl->ren);
		SDL_FillRect(sdl->surf, NULL, 0x00);
		run_with_buff(player, sdl, sdl->win_size.x);
	//	if (t % 2 == 0)
	//	{
			tex = SDL_CreateTextureFromSurface(sdl->ren, sdl->surf);
			sdl_render(sdl->ren, tex, NULL, NULL);
			SDL_DestroyTexture(tex);
	//	}
		SDL_RenderCopy(sdl->ren, text, NULL, &(SDL_Rect){ 20, 20, 150, 55});
		SDL_DestroyTexture(text);
		SDL_RenderPresent(sdl->ren);
		run = hook_event(&player);
	}
	SDL_DestroyTexture(tex);
}


int				main(int argc, char **argv)
{
	t_sector	*sectors;
	t_player	player;
	t_sdl		*sdl;

	if (argc > 1)
		sectors = read_map(argv[1]);
	if (!sectors)
		exit(1);
	sdl = new_t_sdl(W, H, "test_sectors");
	init_sdl(sdl);
	SDL_ShowCursor(SDL_DISABLE);

	player = (t_player){};
	player.pos = (t_vector){3, 3, 0};
	player.half_win_size = (t_point) {sdl->win_size.x / 2, sdl->win_size.y / 2};
//	player.yaw = -0.3;
	// screenHeight / tan(horizontalfov) good value tan(400) 
	player.hfov = sdl->win_size.x / tan(400) /*0.73f * sdl->win_size.y*/; 
//	player.vfov = sdl->win_size.y * (1.0 * .2f);
	// screenWidth / tan(verticalfov) good value tan(350)
	player.vfov	= sdl->win_size.y / tan(350) /*.2f * sdl->win_size.y*/;
//	player.hfov = sdl->win_size.y * (1.0 * 0.73f * sdl->win_size.y / sdl->win_size.x);
	player.height = EyeHeight;
	player.curr_sector = sectors;

	game_loop(sdl, player, sectors);

	list_sectors(sectors);
	
	delete_sectors(sectors);
	free_t_sdl(&sdl);
//	system("leaks -q test");
}