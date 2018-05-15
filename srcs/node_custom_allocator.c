/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   node_custom_allocator.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmickael <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/04/22 17:55:27 by bmickael          #+#    #+#             */
/*   Updated: 2018/04/22 18:11:56 by bmickael         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main_headers.h"

void				*node_custom_allocator(size_t size)
{
	ft_dprintf(B_DEBUG, "{green}NODE ALLOCATION{eoc}\n");
	struct s_node_page		*node_page;
	struct s_node_page		*new_node_page;
	void					*addr;

	node_page = ctx.node_pages_entry;
	if (node_page->primary_block.nb_node == ctx.node_density)
	{
		new_node_page = (struct s_node_page *)
				get_new_pages(NODE_REQ_PAGES * ctx.page_size);
		if (new_node_page == NULL)
			return (NULL);
		new_node_page->primary_block.nb_node = 0;
		new_node_page->primary_block.next = node_page;
		ctx.node_pages_entry = new_node_page;
		node_page = new_node_page;
	}
	addr = &node_page->node[node_page->primary_block.nb_node];
	node_page->primary_block.nb_node += 1;
	(void)size;
	return (addr);
}