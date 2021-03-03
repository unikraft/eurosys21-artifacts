all:
	@./prepare_docker_analysis.sh
	@./run_docker_analysis.sh
	@./generate_heatmap.sh

clean:
	@./clean_docker.sh

plot:
	@./generate_heatmap.sh

prepare:
	@./prepare_docker_analysis.sh

run:
	@./run_docker_analysis.sh

.PHONY:	all clean plot prepare run
