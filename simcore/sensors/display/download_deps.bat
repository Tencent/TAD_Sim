cd Source\Display
set URL=https://raw.githubusercontent.com/kjetil-lye/fractional_brownian_motion/master/fbm/include/fbm
md fbm
cd fbm
curl %URL%/fbm.hpp -o fbm.hpp
curl %URL%/generate_normal_random.hpp -o generate_normal_random.hpp

md impl
cd impl
curl %URL%/impl/data_1d.hpp -o data_1d.hpp
curl %URL%/impl/data_1d_out.hpp -o data_1d_out.hpp
curl %URL%/impl/fbm_inl.hpp -o fbm_inl.hpp
curl %URL%/impl/generate_normal_random_inl.hpp -o generate_normal_random_inl.hpp
curl %URL%/impl/is_power_of_two.hpp -o is_power_of_two.hpp
curl %URL%/impl/variance_fbm_1d.hpp -o variance_fbm_1d.hpp
curl %URL%/impl/data_2d.hpp -o data_2d.hpp
curl %URL%/impl/data_2d_out.hpp -o data_2d_out.hpp
curl %URL%/impl/data_3d.hpp -o data_3d.hpp
curl %URL%/impl/data_3d_out.hpp -o data_3d_out.hpp
curl %URL%/impl/variance_fbm_2d.hpp -o variance_fbm_2d.hpp
curl %URL%/impl/variance_fbm_3d.hpp -o variance_fbm_3d.hpp
