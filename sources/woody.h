/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   woody.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouvel <plouvel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/27 17:57:44 by plouvel           #+#    #+#             */
/*   Updated: 2024/10/28 09:50:10 by plouvel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WOODY_H
#define WOODY_H

#define PAYLOAD_SIZE_OFFSET 0x000001C7
#define KEY_OFFSET 0x000001CF
#define KEY_SIZE 32

extern unsigned char WOODY[];
extern unsigned int WOODY_LEN;

#endif