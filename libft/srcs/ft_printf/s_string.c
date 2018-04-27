/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   s_string.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmickael <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/04/10 19:14:51 by bmickael          #+#    #+#             */
/*   Updated: 2017/04/10 23:57:04 by bmickael         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "internal_printf.h"

static int		get_wchar_t_size(wchar_t *src, int p)
{
	int len;
	int i;

	i = 0;
	len = 0;
	while (*src)
	{
		if (*src <= 0x7F || (*src <= 0xFF && (MB_CUR_MAX == 1)))
			i += 1;
		else if (*src <= 0x7FF)
			i += 2;
		else if (*src <= 0xFFFF)
			i += 3;
		else
			i += 4;
		if ((i > p) && (p != -1))
			break ;
		len = i;
		src++;
	}
	return (len);
}

static int		check_extended_string(wchar_t *src)
{
	int err;

	while (*src)
	{
		err = TRUE;
		if ((*src & 0x80000000) || (*src >= 0xD800 && *src <= 0xDFFF))
			err = TRUE;
		else if (MB_CUR_MAX == 1 && (*src <= (wchar_t)0xFF))
			err = FALSE;
		else if (MB_CUR_MAX == 2 && (*src <= (wchar_t)0x7FF))
			err = FALSE;
		else if (MB_CUR_MAX == 3 && (*src <= (wchar_t)0xFFFF))
			err = FALSE;
		else if (MB_CUR_MAX == 4 && (*src <= (wchar_t)0x10FFFF))
			err = FALSE;
		else
			err = TRUE;
		if (err == TRUE)
			return (0);
		src++;
	}
	return (1);
}

static void		fill_wchar(wchar_t c, char tmp[4], int size)
{
	int i;
	int j;

	if (size == 1)
	{
		tmp[3] = c;
		return ;
	}
	i = 0;
	j = 1;
	while (j < size)
	{
		tmp[3 - i++] = 0x80 | (c & 0b111111);
		c = c >> 6;
		j++;
	}
	if (size == 2)
		tmp[3 - i] = 0xC0 | (c & 0b11111);
	else if (size == 3)
		tmp[3 - i] = 0xE0 | (c & 0b1111);
	else if (size == 4)
		tmp[3 - i] = 0xF0 | (c & 0b111);
}

static int		s_wchar_t(t_args *args, wchar_t *src, t_status *op,
													int *utf8_size)
{
	char	tmp[4];
	int		len;
	int		k;
	int		l;

	if (!(check_extended_string(src)))
		return (-1);
	len = get_wchar_t_size(src, args->p);
	k = (len > args->w) ? len : args->w;
	if (!(args->b & MINUS))
		char_to_buffer((args->b & ZERO) ? '0' : ' ' , k - len, op);
	l = 0;
	while (*src)
	{
		*utf8_size = get_size_for_string(*src);
		fill_wchar(*src, tmp, *utf8_size);
		if ((l += *utf8_size) > len)
			break ;
		string_to_buffer(tmp + (4 - *utf8_size), *utf8_size, op);
		src++;
	}
	if (args->b & MINUS)
		char_to_buffer(' ' , k - len, op);
	return (0);
}

int			s_string(t_args *args, t_status *op)
{
	int			size;
	char		*str;
	wchar_t		*wchar;

	if (args->l & L)
	{
		wchar = (wchar_t *)va_arg(op->ap, wchar_t *);
		return (s_wchar_t(args, (wchar) ? wchar : L"(null)", op, &size));
	}
	str = (char *)va_arg(op->ap, char *);
	str = (!str) ? "(null)" : str;
	size = ft_strlen(str);
	size = ((args->p < size) && (args->p != -1)) ? args->p : size;
	if (args->w <= size)
	{
		string_to_buffer(str, size, op);
		return (0);
	}
	if (args->b & MINUS)
	{
		string_to_buffer(str, size, op);
		char_to_buffer(' ', args->w - size, op);
		return (0);
	}
	char_to_buffer(args->b & ZERO ? '0' : ' ', args->w - size, op);
	string_to_buffer(str, size, op);
	return (0);
}