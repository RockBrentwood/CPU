// Frankenstain Cross-Assemblers, version 2.0.
// Original author: Mark Zenier.
// Switch case actions for unary operators for both the parse and output phase expression evaluators.
		case IFC_NEG:
			etop = -etop;
			break;

		case IFC_NOT:
			etop = ~ etop;
			break;

		case IFC_HIGH:
			etop = (etop >> 8) & 0xff;
			break;

		case IFC_LOW:
			etop = etop & 0xff;

			break;
