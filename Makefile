include ../../SuiteSparse_config/SuiteSparse_config.mk

PLATFORM = X11
HDR = graphics.h easygl_constants.h placer.h solver.h
FLAGS = -D$(PLATFORM) -std=c++11 -DECF -DGRAPHON
GRAPHICS_LIBS = -lX11


assignment:graphics.o block.o placer.o net.o $(HDR)
	g++ $(C) -o assginment.exe $(FLAGS) graphics.o block.o placer.o net.o solver.cpp assignment2.cpp $(LIBS) $(GRAPHICS_LIBS)

graphics.o: $(HDR)
	g++ -c $(FLAGS) graphics.cpp $(GRAPHICS_LIBS)

block.o: $(HDR)
	g++ -c $(FLAGS) block.cpp

placer.o: $(HDR)
	g++ -c $(FLAGS) placer.cpp

net.o: $(HDR)
	g++ -c $(FLAGS) net.cpp

solver.o: $(HDR)
	g++ $(C) $(FLAGS) solver.cpp $(LIBS)

clean:
	rm *.o *.exe



#-------------------------------------------------------------------------------
# UMFPACK optionally uses the CHOLMOD Partition module
LIB_WITH_CHOLMOD =
ifeq (,$(findstring -DNCHOLMOD, $(UMFPACK_CONFIG)))
    LIB_WITH_CHOLMOD = $(LIB_WITH_PARTITION) $(CUBLAS_LIB) $(CUDART_LIB)
endif

#-------------------------------------------------------------------------------

C =$(CF) $(UMFPACK_CONFIG) $(CONFIG_PARTITION) \
    -I../../include

LIBS = $(LDLIBS) -L../../lib -lumfpack -lamd -lsuitesparseconfig \
	$(LIB_WITH_CHOLMOD) $(LAPACK)

libs: metis
	( cd ../../SuiteSparse_config ; $(MAKE) )
	( cd ../../AMD ; $(MAKE) library )
	( cd ../Lib ; $(MAKE) )
	- ( cd ../../CHOLMOD && $(MAKE) library )
	- ( cd ../../COLAMD && $(MAKE) library )
	- ( cd ../../CCOLAMD ; $(MAKE) library )
	- ( cd ../../CAMD ; $(MAKE) library )

metis: ../../include/metis.h

../../include/metis.h:
	- ( cd ../.. && $(MAKE) metis )


