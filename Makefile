build:
	rm -rf build
	mkdir build
	cd build && cmake -DENABLE_AUTH=OFF .. && make -j$$(nproc)

up:
	xhost +local:docker > /dev/null 2>&1 || true
	sudo docker compose up --abort-on-container-exit || true
	sudo docker compose down
