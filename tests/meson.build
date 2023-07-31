criterion = libcriterion.get_variable('criterion')

e_test_header = executable('e_test_header', 'test_header.c', 
                          dependencies: [criterion, nclib],
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test header.', e_test_header)
