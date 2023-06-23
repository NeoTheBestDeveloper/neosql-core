criterion = libcriterion.get_variable('criterion')

e_test_header = executable('e_test_header', 'test_header.c', 
                          dependencies: criterion,
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test header.', e_test_header)

e_test_page = executable('e_test_page', 'test_page.c', 
                          dependencies: criterion,
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test page.', e_test_page)
