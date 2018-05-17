/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deallocator.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmickael <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/04/22 17:47:28 by bmickael          #+#    #+#             */
/*   Updated: 2018/04/22 18:14:49 by bmickael         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main_headers.h"

int		apply_modif(
		struct s_node *record,
		struct s_node *index,
		struct s_couple s[2],
		enum e_page_type type)
{
	btree_delete_rnb_node((struct s_node **)&index->ptr_a,
			record, &node_custom_deallocator);
	if (s[0].len > 0)
		fflush_neighbours(s[0].len, s[0].addr, type);
	if (s[1].len > 0)
		fflush_neighbours(s[1].len, s[1].addr, type);
	return (0);
}

int		inherit_neighbour(
		struct s_node *record,
		struct s_node *index,
		struct s_couple *out,
		enum e_page_type type)
{
	struct s_node	*next;
	struct s_couple	s[2];

	s[0].len = 0;
	s[1].len = 0;
	if ((next = btree_get_next_neighbours_node(record)) != NULL)
	{
		out->len = (uint64_t)next->ptr_a - (uint64_t)record->ptr_a;
		s[0].addr = (void *)((uint64_t)record->ptr_a +
				(uint64_t)record->m.size);
		s[0].len = out->len - record->m.size;
	}
	else if ((uint64_t)record->ptr_a +
			record->m.size != index->m.size + ((type == TINY) ?
			TINY_RANGE : MEDIUM_RANGE))
	{
		s[0].len = (uint64_t)index->m.size + ((type == TINY) ?
				TINY_RANGE : MEDIUM_RANGE) - ((uint64_t)record->ptr_a
						+ record->m.size);
		out->len = s[0].len + record->m.size;
		s[0].addr = (void *)((uint64_t)record->ptr_a +
				(uint64_t)record->m.size);
	}
	do_prev_job(out, &s[1], record, index);
	return (apply_modif(record, index, s, type));
}

void	tiny_medium_deallocate(
		struct s_node *record,
		struct s_node *index,
		enum e_page_type type)
{
	struct s_couple s;

	s.addr = record->ptr_a;
	s.len = record->m.size;
	inherit_neighbour(record, index, &s, type);
	insert_free_record(s.addr, s.len, type, NULL);
}

void	destroy_large_page(struct s_node *record)
{
	destroy_pages(record->ptr_a, record->m.size);
	btree_delete_rnb_node(&ctx.big_page_record_tree,
			record, &node_custom_deallocator);
	return ;
}

void	core_deallocator(void *ptr)
{
	struct s_node		*record;
	struct s_node		*index;
	enum e_page_type	type;

	index = NULL;
	record = btree_get_node_by_content(ctx.big_page_record_tree,
			ptr, &cmp_addr_to_node_addr);
	if (record == NULL)
		index = (struct s_node *)btree_get_node_by_content(
			ctx.index_pages_tree, ptr,
			cmp_addr_to_node_m_addr_range);
	if (record == NULL)
	{
		if (index == NULL)
			return ;
		record = btree_get_node_by_content(index->ptr_a, ptr,
			&cmp_addr_to_node_addr);
	}
	if (record == NULL)
		return ;
	type = get_page_type(record->m.size);
	if (type == LARGE)
		destroy_large_page(record);
	else
		tiny_medium_deallocate(record, index, type);
}
