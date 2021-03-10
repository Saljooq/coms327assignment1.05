Name: Saljooq Altaf
Net ID: saltaf


Assignment 1.04


There were several new functions including initialize_pc, populate_heap, pop_player,push_player_node, kill_player, next_move and kill_all - to accomplish the task of initializing number of monsters with varying characteristics according to the specs. Details are found below.

Four new structs are created as well, including PC, NPC, player_node and player_node_heap. PC and NPC contain limited information on PC and monsters. player_node contain interesting details about all players, including if 'this' node is PC or not, pointer of PC/NPC and concerning details like when_initiated, next_turn etc. The heap is implemented as a linked_list that pop's based on the priorities in the specs (next_turn, when_initiated).

Additionally there is a 2d array 'grid_players' contains all the player's nodes.

1. initialize_pc - this is used to take a pc struct's pointer and create a node and place it in random spot.
2. distant_from_pc - is used to make sure that monsters initialise away from PC.
3. initialize_players - used to create all the npc's as per the specs.
4. populate_heap - populates our heap with all the players.
5. push_player_node - simple method to push different players onto the heap.
6. pop_player - This is used to select a node in the heap based on the specs (lowest -> next_turn, when_initiated)
7. next_move - This is used to figure out what to do with the node selected with pop_player. nextx, and nexty are set to current location of the bot and will determine where it moves next.
This method first checks if bot is tele or not. If it is tele then ifPCseen gets updated to 1, and PCx and PCy get updated to current PC location.
If not tele, then if_in_room updates to see if tele can see the PC.
Then we check to see if bot it intelligent. If intelligent and if PC is seen- then we will use djik algo (based on PCx and PCy and if digger or not) then update nextx and nexty.
If not intelligent, then if PC is seen, then bot moves the nextx, and nexty, horizontally and then vertically towards the location known to the bot.
If the bot is erratic, then we check random, and there's a 50 percent chance neigbouring cell will be used for nextx or nexty, other 50percent chance, next coordinates remain unchanged.
Then the nextx and nexty is processed - as per the specs. First we check if there's a bot sitting on the next coordinates. If there is (and it isn't the current one) then we kill it and remove it from the heap and the grid_players. If there is no bot- action will depend on whether it can tunnel and what the hardness is.
8. kill_player - Used to kill a selected player.
9. kill_all - This is a clean-up mechanism that makes sure all remaining players(malloc'd) are freed.
10. if_in_room- This methods identifies all the non-npc players in the room shared by PC, and updated the ifPCseen to PC's current location PCx and PCy for npc


Additionally, there are some additional lines in main that takes care of getting a '--nummon' switch, with default being 10.
