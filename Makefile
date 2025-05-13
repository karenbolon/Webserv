# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: keramos- <keramos-.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/10/03 11:33:43 by keramos-          #+#    #+#              #
#    Updated: 2025/05/07 17:34:29 by kbolon           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

# **************************************************************************** #
#                                 variables                                    #
# **************************************************************************** #


CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I $(INC_DIR)
RM = rm -rf

#                         Color and Checkmark Definitions                      #

LILAC=\033[1;38;5;183m
PINK=\033[1;38;5;213m
BABY_BLUE = \033[1;38;5;153m
ORG =\033[38;5;216m
BLUE=\033[1;34m
AQUA=\033[1;38;5;79m
AQUA_L=\033[38;5;51m
GREEN=\033[1;32m
CHECK = \033[0;34m✔\033[0m
RT = \033[0m

#                                 Print Function                               #

define print_libs
	@echo "${BABY_BLUE}====> ${1}${RT}"
endef

define print_status
	@echo "${LILAC}====> ${1}${RT}"
endef

define download_libs
	@echo "${ORG}====> ${1}${RT}"
endef

# **************************************************************************** #
#                                 SOURCES                                      #
# **************************************************************************** #

SRC_DIR = srcs
INC_DIR = includes
OBJ_DIR = objs

SRCS = 	$(SRC_DIR)/WebServ.cpp \
		$(SRC_DIR)/ServerSocket.cpp $(SRC_DIR)/ClientConnection.cpp $(SRC_DIR)/helperFunction.cpp \
		$(SRC_DIR)/ConfigParser.cpp $(SRC_DIR)/LocationConfig.cpp $(SRC_DIR)/ServerConfig.cpp \
		$(SRC_DIR)/Util.cpp


#patsubst is short for pattern substitution, works with items in multiple folders
OBJS = $(notdir $(SRCS:.cpp=.o))
OBJS := $(patsubst %, $(OBJ_DIR)/%,$(OBJS))

# **************************************************************************** #
#                                  RULES                                       #
# **************************************************************************** #

all: Start $(NAME) client End

Start :
	@if [ -f $(NAME) ]; then \
		echo "${LILAC}Checking if build is necessary...${RT}"; \
	else \
		echo "${PINK}Compiling WebServ...${RT}"; \
		echo "Starting the build process...!"; \
	fi

$(NAME): $(OBJS)
	$(call print_status,"Creating WebServ...")
	@$(CXX) $(CXXFLAGS) -o $@ $^ > /dev/null
	@echo "${CHECK} Compiling utilities! ${RT}"

client: $(SRC_DIR)/test_client.cpp $(SRC_DIR)/ConfigParser.cpp $(SRC_DIR)/helperFunction.cpp \
	$(SRC_DIR)/LocationConfig.cpp $(SRC_DIR)/ServerConfig.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $^ > /dev/null

create_obj_dir:
	@mkdir -p $(OBJ_DIR)
	@echo "${CHECK} Object directory created!"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | create_obj_dir
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@ > /dev/null

End :
	@echo "${PINK}WebServ...${RT}";
	@echo "${CHECK} successfully compiled!         🎉$(RT)";

# **************************************************************************** #
#                                clean RULES                                   #
# **************************************************************************** #

clean:
	@echo "${ORG}==> Cleaning up object files...${RT}"
	@$(RM) $(OBJ_DIR)
	@echo "${CHECK} Cleanup complete               🧹"

fclean: clean
	@echo "${ORG}==> Full clean - Removing binaries...${RT}"
	@$(RM) $(NAME) client
	@echo "${CHECK} Full cleanup complete          🧹"

re: fclean all

.PHONY: all clean fclean re