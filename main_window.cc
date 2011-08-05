#include "main_window.h"

void main_window::sample_double_clicked(const QModelIndex &index) {
	engine_.commands.write(boost::bind(&main_window::print_foo, this));
	try {
		disposable_generator_ptr p = disposable_generator::create(
			generator(
				disposable_sample::create(
					sample(std::string(file_system_model.filePath(index).toLatin1()))
				)
			)
		);
		if(engine_.commands.can_write()) {
			std::cout << "writing command" << std::endl;
			engine_.commands.write(assign(engine_.gens->t[0], p));
		} 
		else {
			std::cout << "full" << std::endl; 
		}
	} catch (...) {
		std::cout << "something went wrong" << std::endl;
	}
}