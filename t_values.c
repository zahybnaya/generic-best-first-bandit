 
double t_values[] = {12.706205, 4.302653, 3.182446, 2.776445, 2.570582, 2.446912, 2.364624, 2.306004, 2.262157, 2.228139, 2.200985, 2.178813, 2.160369, 2.144787, 2.131450, 2.119905, 2.109816, 2.100922, 2.093024, 2.085963, 2.079614, 2.073873, 2.068658, 2.063899, 2.059539, 2.055529, 2.051831, 2.048407, 2.045230, 2.042272, 2.039513, 2.036933, 2.034515, 2.032245, 2.030108, 2.028094, 2.026192, 2.024394, 2.022691, 2.021075, 2.019541, 2.018082, 2.016692, 2.015368, 2.014103, 2.012896, 2.011741, 2.010635, 2.009575, 2.008559, 2.007584, 2.006647, 2.005746, 2.004879, 2.004045, 2.003241, 2.002465, 2.001717, 2.000995, 2.000298, 1.999624, 1.998972, 1.998341, 1.997730, 1.997138, 1.996564, 1.996008, 1.995469, 1.994945, 1.994437, 1.993943, 1.993464, 1.992997, 1.992543, 1.992102, 1.991673, 1.991254, 1.990847, 1.990450, 1.990063, 1.989686, 1.989319, 1.988960, 1.988610, 1.988268, 1.987934, 1.987608, 1.987290, 1.986979, 1.986675, 1.986377, 1.986086, 1.985802, 1.985523, 1.985251, 1.984984, 1.984723, 1.984467, 1.984217, 1.983972, 1.983731, 1.983495, 1.983264, 1.983038, 1.982815, 1.982597, 1.982383, 1.982173, 1.981967, 1.981765, 1.981567, 1.981372, 1.981180, 1.980992, 1.980808, 1.980626, 1.980448, 1.980272, 1.980100, 1.979930, 1.979764, 1.979600, 1.979439, 1.979280, 1.979124, 1.978971, 1.978820, 1.978671, 1.978524, 1.978380, 1.978239, 1.978099, 1.977961, 1.977826, 1.977692, 1.977561, 1.977431, 1.977304, 1.977178, 1.977054, 1.976931, 1.976811, 1.976692, 1.976575, 1.976460, 1.976346, 1.976233, 1.976122, 1.976013, 1.975905, 1.975799, 1.975694, 1.975590, 1.975488, 1.975387, 1.975288, 1.975189, 1.975092, 1.974996, 1.974902, 1.974808, 1.974716, 1.974625, 1.974535, 1.974446, 1.974358, 1.974271, 1.974185, 1.974100, 1.974017, 1.973934, 1.973852, 1.973771, 1.973691, 1.973612, 1.973534, 1.973457, 1.973381, 1.973305, 1.973231, 1.973157, 1.973084, 1.973012, 1.972941, 1.972870, 1.972800, 1.972731, 1.972663, 1.972595, 1.972528, 1.972462, 1.972396, 1.972332, 1.972268, 1.972204, 1.972141, 1.972079, 1.972017, 1.971957, 1.971896, 1.971837, 1.971777, 1.971719, 1.971661, 1.971603, 1.971547, 1.971490, 1.971435, 1.971379, 1.971325, 1.971271, 1.971217, 1.971164, 1.971111, 1.971059, 1.971007, 1.970956, 1.970906, 1.970855, 1.970806, 1.970756, 1.970707, 1.970659, 1.970611, 1.970563, 1.970516, 1.970470, 1.970423, 1.970377, 1.970332, 1.970287, 1.970242, 1.970198, 1.970154, 1.970110, 1.970067, 1.970024, 1.969982, 1.969939, 1.969898, 1.969856, 1.969815, 1.969774, 1.969734, 1.969694, 1.969654, 1.969615, 1.969576, 1.969537, 1.969498, 1.969460, 1.969422, 1.969385, 1.969348, 1.969311, 1.969274, 1.969237, 1.969201, 1.969166, 1.969130, 1.969095, 1.969060, 1.969025, 1.968990, 1.968956, 1.968922, 1.968889, 1.968855, 1.968822, 1.968789, 1.968756, 1.968724, 1.968692, 1.968660, 1.968628, 1.968596, 1.968565, 1.968534, 1.968503, 1.968472, 1.968442, 1.968412, 1.968382, 1.968352, 1.968323, 1.968293, 1.968264, 1.968235, 1.968206, 1.968178, 1.968150, 1.968121, 1.968093, 1.968066, 1.968038, 1.968011, 1.967984, 1.967957, 1.967930, 1.967903, 1.967877, 1.967850, 1.967824, 1.967798, 1.967772, 1.967747, 1.967721, 1.967696, 1.967671, 1.967646, 1.967621, 1.967596, 1.967572, 1.967548, 1.967524, 1.967500, 1.967476, 1.967452, 1.967428, 1.967405, 1.967382, 1.967359, 1.967336, 1.967313, 1.967290, 1.967268, 1.967245, 1.967223, 1.967201, 1.967179, 1.967157, 1.967135, 1.967113, 1.967092, 1.967071, 1.967049, 1.967028, 1.967007, 1.966986, 1.966966, 1.966945, 1.966925, 1.966904, 1.966884, 1.966864, 1.966844, 1.966824, 1.966804, 1.966785, 1.966765, 1.966746, 1.966726, 1.966707, 1.966688, 1.966669, 1.966650, 1.966631, 1.966613, 1.966594, 1.966575, 1.966557, 1.966539, 1.966521, 1.966503, 1.966485, 1.966467, 1.966449, 1.966431, 1.966414, 1.966396, 1.966379, 1.966362, 1.966344, 1.966327, 1.966310, 1.966293, 1.966276, 1.966260, 1.966243, 1.966226, 1.966210, 1.966194, 1.966177, 1.966161, 1.966145, 1.966129, 1.966113, 1.966097, 1.966081, 1.966065, 1.966050, 1.966034, 1.966019, 1.966003, 1.965988, 1.965973, 1.965957, 1.965942, 1.965927, 1.965912, 1.965897, 1.965883, 1.965868, 1.965853, 1.965839, 1.965824, 1.965810, 1.965795, 1.965781, 1.965767, 1.965753, 1.965739, 1.965725, 1.965711, 1.965697, 1.965683, 1.965669, 1.965655, 1.965642, 1.965628, 1.965615, 1.965601, 1.965588, 1.965575, 1.965561, 1.965548, 1.965535, 1.965522, 1.965509, 1.965496, 1.965483, 1.965471, 1.965458, 1.965445, 1.965432, 1.965420, 1.965407, 1.965395, 1.965382, 1.965370, 1.965358, 1.965346, 1.965333, 1.965321, 1.965309, 1.965297, 1.965285, 1.965273, 1.965261, 1.965250, 1.965238, 1.965226, 1.965215, 1.965203, 1.965191, 1.965180, 1.965168, 1.965157, 1.965146, 1.965134, 1.965123, 1.965112, 1.965101, 1.965090, 1.965079, 1.965068, 1.965057, 1.965046, 1.965035, 1.965024, 1.965013, 1.965003, 1.964992, 1.964981, 1.964971, 1.964960, 1.964950, 1.964939, 1.964929, 1.964918, 1.964908, 1.964898, 1.964888, 1.964877, 1.964867, 1.964857, 1.964847, 1.964837, 1.964827, 1.964817, 1.964807, 1.964797, 1.964788, 1.964778, 1.964768, 1.964758, 1.964749, 1.964739, 1.964729, 1.964720, 1.964710, 1.964701, 1.964691, 1.964682, 1.964673, 1.964663, 1.964654, 1.964645, 1.964636, 1.964626, 1.964617, 1.964608, 1.964599, 1.964590, 1.964581, 1.964572, 1.964563, 1.964554, 1.964545, 1.964537, 1.964528, 1.964519, 1.964510, 1.964502, 1.964493, 1.964484, 1.964476, 1.964467, 1.964459, 1.964450, 1.964442, 1.964433, 1.964425, 1.964416, 1.964408, 1.964400, 1.964391, 1.964383, 1.964375, 1.964367, 1.964359, 1.964350, 1.964342, 1.964334, 1.964326, 1.964318, 1.964310, 1.964302, 1.964294, 1.964287, 1.964279, 1.964271, 1.964263, 1.964255, 1.964248, 1.964240, 1.964232, 1.964224, 1.964217, 1.964209, 1.964202, 1.964194, 1.964187, 1.964179, 1.964172, 1.964164, 1.964157, 1.964149, 1.964142, 1.964135, 1.964127, 1.964120, 1.964113, 1.964105, 1.964098, 1.964091, 1.964084, 1.964077, 1.964070, 1.964063, 1.964055, 1.964048, 1.964041, 1.964034, 1.964027, 1.964020, 1.964014, 1.964007, 1.964000, 1.963993, 1.963986, 1.963979, 1.963972, 1.963966, 1.963959, 1.963952, 1.963946, 1.963939, 1.963932, 1.963926, 1.963919, 1.963912, 1.963906, 1.963899, 1.963893, 1.963886, 1.963880, 1.963873, 1.963867, 1.963861, 1.963854, 1.963848, 1.963841, 1.963835, 1.963829, 1.963823, 1.963816, 1.963810, 1.963804, 1.963798, 1.963791, 1.963785, 1.963779, 1.963773, 1.963767, 1.963761, 1.963755, 1.963749, 1.963743, 1.963737, 1.963731, 1.963725, 1.963719, 1.963713, 1.963707, 1.963701, 1.963695, 1.963689, 1.963683, 1.963678, 1.963672, 1.963666, 1.963660, 1.963654, 1.963649, 1.963643, 1.963637, 1.963632, 1.963626, 1.963620, 1.963615, 1.963609, 1.963603, 1.963598, 1.963592, 1.963587, 1.963581, 1.963576, 1.963570, 1.963565, 1.963559, 1.963554, 1.963549, 1.963543, 1.963538, 1.963532, 1.963527, 1.963522, 1.963516, 1.963511, 1.963506, 1.963500, 1.963495, 1.963490, 1.963485, 1.963479, 1.963474, 1.963469, 1.963464, 1.963459, 1.963454, 1.963448, 1.963443, 1.963438, 1.963433, 1.963428, 1.963423, 1.963418, 1.963413, 1.963408, 1.963403, 1.963398, 1.963393, 1.963388, 1.963383, 1.963378, 1.963373, 1.963368, 1.963364, 1.963359, 1.963354, 1.963349, 1.963344, 1.963339, 1.963335, 1.963330, 1.963325, 1.963320, 1.963316, 1.963311, 1.963306, 1.963301, 1.963297, 1.963292, 1.963287, 1.963283, 1.963278, 1.963273, 1.963269, 1.963264, 1.963260, 1.963255, 1.963251, 1.963246, 1.963241, 1.963237, 1.963232, 1.963228, 1.963223, 1.963219, 1.963215, 1.963210, 1.963206, 1.963201, 1.963197, 1.963192, 1.963188, 1.963184, 1.963179, 1.963175, 1.963171, 1.963166, 1.963162, 1.963158, 1.963153, 1.963149, 1.963145, 1.963141, 1.963136, 1.963132, 1.963128, 1.963124, 1.963119, 1.963115, 1.963111, 1.963107, 1.963103, 1.963099, 1.963094, 1.963090, 1.963086, 1.963082, 1.963078, 1.963074, 1.963070, 1.963066, 1.963062, 1.963058, 1.963054, 1.963050, 1.963046, 1.963042, 1.963038, 1.963034, 1.963030, 1.963026, 1.963022, 1.963018, 1.963014, 1.963010, 1.963006, 1.963002, 1.962998, 1.962994, 1.962991, 1.962987, 1.962983, 1.962979, 1.962975, 1.962971, 1.962968, 1.962964, 1.962960, 1.962956, 1.962952, 1.962949, 1.962945, 1.962941, 1.962937, 1.962934, 1.962930, 1.962926, 1.962923, 1.962919, 1.962915, 1.962912, 1.962908, 1.962904, 1.962901, 1.962897, 1.962893, 1.962890, 1.962886, 1.962883, 1.962879, 1.962875, 1.962872, 1.962868, 1.962865, 1.962861, 1.962858, 1.962854, 1.962851, 1.962847, 1.962844, 1.962840, 1.962837, 1.962833, 1.962830, 1.962826, 1.962823, 1.962819, 1.962816, 1.962812, 1.962809, 1.962806, 1.962802, 1.962799, 1.962795, 1.962792, 1.962789, 1.962785, 1.962782, 1.962779, 1.962775, 1.962772, 1.962769, 1.962765, 1.962762, 1.962759, 1.962756, 1.962752, 1.962749, 1.962746, 1.962742, 1.962739, 1.962736, 1.962733, 1.962729, 1.962726, 1.962723, 1.962720, 1.962717, 1.962713, 1.962710, 1.962707, 1.962704, 1.962701, 1.962698, 1.962694, 1.962691, 1.962688, 1.962685, 1.962682, 1.962679, 1.962676, 1.962673, 1.962670, 1.962666, 1.962663, 1.962660, 1.962657, 1.962654, 1.962651, 1.962648, 1.962645, 1.962642, 1.962639, 1.962636, 1.962633, 1.962630, 1.962627, 1.962624, 1.962621, 1.962618, 1.962615, 1.962612, 1.962609, 1.962606, 1.962603, 1.962600, 1.962597, 1.962595, 1.962592, 1.962589, 1.962586, 1.962583, 1.962580, 1.962577, 1.962574, 1.962571, 1.962569, 1.962566, 1.962563, 1.962560, 1.962557, 1.962554, 1.962552, 1.962549, 1.962546, 1.962543, 1.962540, 1.962537, 1.962535, 1.962532, 1.962529, 1.962526, 1.962524, 1.962521, 1.962518, 1.962515, 1.962513, 1.962510, 1.962507, 1.962504, 1.962502, 1.962499, 1.962496, 1.962494, 1.962491, 1.962488, 1.962486, 1.962483, 1.962480, 1.962477, 1.962475, 1.962472, 1.962470, 1.962467, 1.962464, 1.962462, 1.962459, 1.962456, 1.962454, 1.962451, 1.962449, 1.962446, 1.962443, 1.962441, 1.962438, 1.962436, 1.962433, 1.962430, 1.962428, 1.962425, 1.962423, 1.962420, 1.962418, 1.962415, 1.962413, 1.962410, 1.962408, 1.962405, 1.962403, 1.962400, 1.962398, 1.962395, 1.962393, 1.962390, 1.962388, 1.962385, 1.962383, 1.962380, 1.962378, 1.962375, 1.962373, 1.962370, 1.962368, 1.962366, 1.962363, 1.962361, 1.962358, 1.962356, 1.962353, 1.962351, 1.962349, 1.962346, 1.962344, 1.962341, 1.962339, 1.962337, 1.962334, 1.962332, 1.962330, 1.962327, 1.962325, 1.962323, 1.962320, 1.962318, 1.962316, 1.962313, 1.962311, 1.962309, 1.962306, 1.962304, 1.962302, 1.962299, 1.962297, 1.962295, 1.962292, 1.962290, 1.962288, 1.962286, 1.962283, 1.962281, 1.962279, 1.962277, 1.962274, 1.962272, 1.962270, 1.962268, 1.962265, 1.962263, 1.962261, 1.962259, 1.962256, 1.962254, 1.962252, 1.962250, 1.962248, 1.962245, 1.962243, 1.962241, 1.962239, 1.962237, 1.962235, 1.962232, 1.962230, 1.962228, 1.962226, 1.962224, 1.962222, 1.962219, 1.962217, 1.962215, 1.962213, 1.962211, 1.962209, 1.962207, 1.962204, 1.962202, 1.962200, 1.962198, 1.962196, 1.962194, 1.962192, 1.962190, 1.962188, 1.962186, 1.962184, 1.962181, 1.962179, 1.962177, 1.962175, 1.962173, 1.962171, 1.962169, 1.962167, 1.962165, 1.962163, 1.962161, 1.962159, 1.962157, 1.962155, 1.962153, 1.962151, 1.962149, 1.962147, 1.962145, 1.962143, 1.962141, 1.962139, 1.962137, 1.962135, 1.962133, 1.962131, 1.962129, 1.962127, 1.962125, 1.962123, 1.962121, 1.962119, 1.962117, 1.962115, 1.962113, 1.962111, 1.962109, 1.962107, 1.962105, 1.962103, 1.962102, 1.962100, 1.962098, 1.962096, 1.962094, 1.962092, 1.962090, 1.962088, 1.962086, 1.962084, 1.962082, 1.962081, 1.962079, 1.962077, 1.962075, 1.962073, 1.962071, 1.962069, 1.962067, 1.962066, 1.962064, 1.962062, 1.962060, 1.962058, 1.962056, 1.962054, 1.962053, 1.962051, 1.962049, 1.962047, 1.962045, 1.962043, 1.962042, 1.962040, 1.962038, 1.962036, 1.962034, 1.962033, 1.962031, 1.962029, 1.962027, 1.962025, 1.962024, 1.962022, 1.962020, 1.962018, 1.962016, 1.962015, 1.962013, 1.962011, 1.962009, 1.962008, 1.962006, 1.962004, 1.962002, 1.962001, 1.961999, 1.961997, 1.961995, 1.961994, 1.961992, 1.961990, 1.961988, 1.961987, 1.961985, 1.961983, 1.961982, 1.961980, 1.961978, 1.961976, 1.961975, 1.961973, 1.961971, 1.961970, 1.961968, 1.961966, 1.961965, 1.961963, 1.961961, 1.961959, 1.961958, 1.961956, 1.961954, 1.961953, 1.961951, 1.961949, 1.961948, 1.961946, 1.961944, 1.961943, 1.961941, 1.961940, 1.961938, 1.961936, 1.961935, 1.961933, 1.961931, 1.961930, 1.961928, 1.961926, 1.961925, 1.961923, 1.961922, 1.961920, 1.961918, 1.961917, 1.961915, 1.961914, 1.961912, 1.961910, 1.961909, 1.961907, 1.961906, 1.961904, 1.961902, 1.961901, 1.961899, 1.961898, 1.961896, 1.961895, 1.961893, 1.961891, 1.961890, 1.961888, 1.961887, 1.961885, 1.961884, 1.961882, 1.961880, 1.961879, 1.961877, 1.961876, 1.961874, 1.961873, 1.961871, 1.961870, 1.961868, 1.961867, 1.961865, 1.961864, 1.961862, 1.961861, 1.961859, 1.961858, 1.961856, 1.961855, 1.961853, 1.961852, 1.961850, 1.961849, 1.961847, 1.961846, 1.961844, 1.961843, 1.961841, 1.961840, 1.961838, 1.961837, 1.961835, 1.961834, 1.961832, 1.961831, 1.961829, 1.961828, 1.961826, 1.961825, 1.961823, 1.961822, 1.961820, 1.961819, 1.961818, 1.961816, 1.961815, 1.961813, 1.961812, 1.961810, 1.961809, 1.961808, 1.961806, 1.961805, 1.961803, 1.961802, 1.961800, 1.961799, 1.961798, 1.961796, 1.961795, 1.961793, 1.961792, 1.961790, 1.961789, 1.961788, 1.961786, 1.961785, 1.961783, 1.961782, 1.961781, 1.961779, 1.961778, 1.961777, 1.961775, 1.961774, 1.961772, 1.961771, 1.961770, 1.961768, 1.961767, 1.961766, 1.961764, 1.961763, 1.961761, 1.961760, 1.961759, 1.961757, 1.961756, 1.961755, 1.961753, 1.961752, 1.961751, 1.961749, 1.961748, 1.961747, 1.961745, 1.961744, 1.961743, 1.961741, 1.961740, 1.961739, 1.961737, 1.961736, 1.961735, 1.961733, 1.961732, 1.961731, 1.961729, 1.961728, 1.961727, 1.961725, 1.961724, 1.961723, 1.961721, 1.961720, 1.961719, 1.961718, 1.961716, 1.961715, 1.961714, 1.961712, 1.961711, 1.961710, 1.961709, 1.961707, 1.961706, 1.961705, 1.961703, 1.961702, 1.961701, 1.961700, 1.961698, 1.961697, 1.961696, 1.961695, 1.961693, 1.961692, 1.961691, 1.961690, 1.961688, 1.961687, 1.961686, 1.961685, 1.961683, 1.961682, 1.961681, 1.961680, 1.961678, 1.961677, 1.961676, 1.961675, 1.961673, 1.961672, 1.961671, 1.961670, 1.961668, 1.961667, 1.961666, 1.961665, 1.961664, 1.961662, 1.961661, 1.961660, 1.961659, 1.961657, 1.961656, 1.961655, 1.961654, 1.961653, 1.961651, 1.961650, 1.961649, 1.961648, 1.961647, 1.961645, 1.961644, 1.961643, 1.961642, 1.961641, 1.961640, 1.961638, 1.961637, 1.961636, 1.961635, 1.961634, 1.961632, 1.961631, 1.961630, 1.961629, 1.961628, 1.961627, 1.961625, 1.961624, 1.961623, 1.961622, 1.961621, 1.961620, 1.961619, 1.961617, 1.961616, 1.961615, 1.961614, 1.961613, 1.961612, 1.961610, 1.961609, 1.961608, 1.961607, 1.961606, 1.961605, 1.961604, 1.961603, 1.961601, 1.961600, 1.961599, 1.961598, 1.961597, 1.961596, 1.961595, 1.961594, 1.961592, 1.961591, 1.961590, 1.961589, 1.961588, 1.961587, 1.961586, 1.961585, 1.961583, 1.961582, 1.961581, 1.961580, 1.961579, 1.961578, 1.961577, 1.961576, 1.961575, 1.961574, 1.961573, 1.961571, 1.961570, 1.961569, 1.961568, 1.961567, 1.961566, 1.961565, 1.961564, 1.961563, 1.961562, 1.961561, 1.961560, 1.961558, 1.961557, 1.961556, 1.961555, 1.961554, 1.961553, 1.961552, 1.961551, 1.961550, 1.961549, 1.961548, 1.961547};