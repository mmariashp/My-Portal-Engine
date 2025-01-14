#include "main_head.h"

t_sector			*new_sector()
{
	t_sector		*sec;
	int				i;

	i = 0;
	sec = (t_sector*)malloc(sizeof(t_sector));
	*sec = (t_sector){};
	while (i < MAX_PORTALS)
		sec->portals[i++] = -1;
	printf("Created\n");
	return (sec);
}

t_sector			*crate_new_sector(float	floor, float ceil)
{
	t_sector		*sec;

	sec = new_sector();
	sec->floor = floor;
	sec->ceil = ceil;
	return (sec);
}

int					size_sector(t_sector *sec)
{
	int				i;
	t_sector		*s;

	s = sec;
	i = 0;
	while(s)
	{
		s = s->next;
		i++;
	}
	return (i);
}

void				add_next_sector(t_sector **main, t_sector *next)
{
	t_sector		*head;
	int				i;

	head = *main;
	if (!head)
		return ;
	i = 1;
	while (head->next)
	{
		head = head->next;
		i++;
	}
	next->sector = i;
	head->next = next;
/*	next->next = *main;
	next->sector = size_sector(next);
	*main = next;*/
}

void				delete_walls(t_wall **wall, unsigned size)
{
	int				i;
	t_wall			*ptr;
	
	if (!wall)
		return ;
	i = 0;
	while (i < size)
	{
		if (wall[i])
		{
			ptr = wall[i];
			ft_memdel((void**)&ptr);
			wall[i] = NULL;
		}
		i++;
	}
	ft_memdel((void**)&wall);
}

void				delete_sectors(t_sector *sectors)
{
	t_sector		*next;
	int				i = 0;

	if (!sectors)
		return ;
	next = sectors;
	while(sectors)
	{
		i++;
		if (i > 10)
			break ;
		next = sectors;
		sectors = sectors->next;
		delete_walls(next->wall, next->n_walls);
		ft_memdel((void**)&next);
		printf("Destroyed\n");
	}
}

void				list_walls(t_wall **walls, int size)
{
	t_wall			*w;
	t_vector		line;
	int				i;

	i = 0;
	while (i < size)
	{
		w = walls[i];
		ft_putstr("\twall id ");
		ft_putnbr(walls[i]->id);
		ft_putstr(", type ");
		ft_putnbr(walls[i]->type);
		ft_putstr(". Neighbors:");
		if (walls[i]->sectors[0])
		{
			ft_putstr(" sect# ");
			ft_putnbr(walls[i]->sectors[0]->sector);
		}
		if (walls[i]->sectors[1])
		{
			ft_putstr(", and  ");
			ft_putnbr(walls[i]->sectors[1]->sector);
		}
		line = w->start;
		ft_putstr("\n\t start: x = ");
		ft_putnbr(line.x);
		ft_putstr(", y = ");
		ft_putnbr(line.y);
		line = w->end;
		ft_putstr(";\n\t end: x = ");
		ft_putnbr(line.x);
		ft_putstr(", y = ");
		ft_putnbr(line.y);
		write(1, ".\n", 2);
		
		i++;
	}
}

void				list_sectors(t_sector *head)
{
	t_sector		*s;
	int				i;

	if (!head)
		return ;
	s = head;
	i = 0;
	while (s)
	{
		ft_putstr("Sector # ");
		ft_putnbr(i);
		ft_putstr("\ncount walls: ");
		ft_putnbr(s->n_walls);
		ft_putstr("\nfloor height: ");
		ft_putnbr(s->floor);
		ft_putstr(", ceil height: ");
		ft_putnbr(s->ceil);
		write(1, "\n", 1);
		list_walls(s->wall, s->n_walls);
		i++;
		s = s->next;
	}
}