criterion = libcriterion.get_variable('criterion')

e_test_header = executable('e_test_header', 'test_header.c', 
                          dependencies: [criterion, nclib],
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test header.', e_test_header)

e_test_page = executable('e_test_page', 'test_page.c', 
                          dependencies: [criterion, nclib],
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test page.', e_test_page)

e_test_block = executable('e_test_block', 'test_block.c', 
                          dependencies: [criterion, nclib],
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test block.', e_test_block)
