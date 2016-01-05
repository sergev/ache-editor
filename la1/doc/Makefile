#
#	Copyright abandoned, 1983, The Rand Corporation
#


Doc : Doc.n
	-mv Doc ,Doc
	nroff -ms Doc.n | page > Doc
#       fix up Doc by hand:
#         indent extra lines in .Ia and .Ba macros
#         fix widoes

Doc.n : Doc1.n Doc2.n
	touch Doc.n

Doc1.n : Doc1.tbl
	tbl Doc1.tbl > Doc1.n

Doc2.n : Doc2.tbl
	tbl Doc2.tbl > Doc2.n
