import struct
from dataclasses import dataclass
from enum import Enum
from io import BytesIO
from typing import Any, Final, NoReturn, TypeAlias

PAGE_SIZE: Final[int] = 4096
PAGE_HEADER_SIZE: Final[int] = 32
PAGE_PAYLOAD_SIZE: Final[int] = PAGE_SIZE - PAGE_HEADER_SIZE

BLOCK_HEADER_SIZE: Final[int] = 15

VALID_DB_PATH: Final[str] = "database.db"

NULL_ADDR: Final[tuple[int, int]] = (-1, -1)


def pack_chars(string: str) -> bytes:
    return string.encode()


def pack_bytes(fmt: str, *values: Any) -> bytes:
    return struct.pack(f"<{fmt}", values[0])


def pack_bool(flag: bool) -> bytes:
    return pack_bytes("?", flag)


def pack_u8(number: int) -> bytes:
    return pack_bytes("B", number)


def pack_i16(number: int) -> bytes:
    return pack_bytes("h", number)


def pack_u16(number: int) -> bytes:
    return pack_bytes("H", number)


def pack_i32(number: int) -> bytes:
    return pack_bytes("i", number)


def pack_u32(number: int) -> bytes:
    return pack_bytes("I", number)


def pack_u64(number: int) -> bytes:
    return pack_bytes("Q", number)


def pack_i64(number: int) -> bytes:
    return pack_bytes("q", number)


def pack_addr(addr: tuple[int, int]) -> bytes:
    return pack_i32(addr[0]) + pack_i16(addr[1])


class DataTypeId(int, Enum):
    CHAR = 0
    VARCHAR = 1
    TEXT = 2
    MICROINT = 3
    SMALLINT = 4
    INT = 5
    BIGINT = 6
    BOOL = 7
    REAL = 8
    TIMESTAMP = 9


@dataclass
class DataType:
    id_: DataTypeId
    max_size: int
    nullable: bool

    def __bytes__(self) -> bytes:
        buf = BytesIO()
        buf.write(pack_u8(self.id_))
        buf.write(pack_u16(self.max_size))
        buf.write(pack_bool(self.nullable))

        return buf.getvalue()


def pack_varchar(s: str | None, nullable: bool) -> bytes:
    if not nullable and s is not None:
        if len(s) <= 255:
            return pack_u8(len(s)) + pack_chars(s)
        return pack_u16(len(s)) + pack_chars(s)

    if s is None:
        return pack_bool(True)

    if len(s) <= 255:
        return pack_bool(False) + pack_u8(len(s)) + pack_chars(s)
    return pack_bool(False) + pack_u16(len(s)) + pack_chars(s)


def pack_bigint(value: int | None, nullable: bool) -> bytes | NoReturn:
    if value is None:
        return pack_bool(True)

    if nullable:
        return pack_bool(False) + pack_i64(value)

    return pack_i64(value)


def pack_by_type(data_type: DataType, value: Any | None) -> bytes | NoReturn:
    if not data_type.nullable and value is None:
        raise TypeError("Trying set null for not null type.")

    match data_type.id_:
        case DataTypeId.VARCHAR:
            if isinstance(value, str) or value is None:
                return pack_varchar(value, data_type.nullable)
            raise TypeError('Expected type "str"')

        case DataTypeId.BIGINT:
            if isinstance(value, int) or value is None:
                return pack_bigint(value, data_type.nullable)
            raise TypeError('Expected type "int"')

        case _:
            raise TypeError(f"Unxpected type {data_type}")


@dataclass
class Column:
    type_: DataType
    name: str

    def __bytes__(self) -> bytes:
        buf = BytesIO()
        buf.write(bytes(self.type_))
        buf.write(pack_u8(len(self.name)))
        buf.write(pack_chars(self.name))

        return buf.getvalue()


@dataclass
class Table:
    first_record: tuple[int, int]
    last_record: tuple[int, int]
    records_count: int
    name: str
    columns: list[Column]

    def __bytes__(self) -> bytes:
        buf = BytesIO()
        buf.write(pack_addr(self.first_record))
        buf.write(pack_addr(self.last_record))
        buf.write(pack_u8(len(self.columns)))
        buf.write(pack_u64(self.records_count))
        buf.write(pack_u8(len(self.name)))
        buf.write(pack_chars(self.name))

        for column in self.columns:
            buf.write(bytes(column))

        return buf.getvalue()


RecordSupportedType: TypeAlias = str | int


class Record:
    table: Table
    values: list[RecordSupportedType | None]

    def __init__(self, table: Table, values: list[RecordSupportedType | None]) -> None:
        self.table = table
        self.values = values.copy()

    def __bytes__(self) -> bytes:
        buf = BytesIO()

        data_types = (column.type_ for column in self.table.columns)

        for value, data_type in zip(self.values, data_types):
            buf.write(pack_by_type(data_type, value))

        return bytes(buf.getbuffer())


@dataclass
class BlockHeader:
    payload_size: int
    next: tuple[int, int]
    parted: bool

    def __bytes__(self) -> bytes:
        buf = BytesIO()
        buf.write(pack_u64(self.payload_size))
        buf.write(pack_addr(self.next))
        buf.write(pack_bool(self.parted))
        return buf.getvalue()


class Block:
    header: BlockHeader
    payload: bytes

    def __init__(self, payload: bytes) -> None:
        if len(payload) > PAGE_PAYLOAD_SIZE:
            raise ValueError(
                "Payload size is higher then PAGE_PAYLOAD_SIZE."
                "Use for this payload PartedBlock class."
            )
        self.payload = payload

        self.header = BlockHeader(
            payload_size=len(self.payload), next=(0, 0), parted=False
        )

    def __bytes__(self) -> bytes | NoReturn:
        return bytes(self.header) + self.payload


@dataclass
class BlockPartHeader:
    payload_size: int
    next: tuple[int, int]

    def __bytes__(self) -> bytes:
        buf = BytesIO()
        buf.write(pack_u64(self.payload_size))
        buf.write(pack_addr(self.next))
        return buf.getvalue()


class BlockPart:
    header: BlockPartHeader
    payload: bytes

    def __init__(self, payload: bytes) -> None:
        self.payload = payload
        self.header = BlockPartHeader(payload_size=len(self.payload), next=(0, 0))

    def __bytes__(self) -> bytes:
        return bytes(self.header) + self.payload


class PartedBlock:
    header: BlockHeader
    parts: list[BlockPart]

    def __init__(self, payload: bytes) -> None:
        self.parts = []
        self.header = BlockHeader(payload_size=len(payload), next=(0, 0), parted=True)

        # First part
        first_part_size = PAGE_PAYLOAD_SIZE - 14 - 15
        self.parts.append(BlockPart(payload[:first_part_size]))

        remain = payload[first_part_size:]
        max_part_size = PAGE_PAYLOAD_SIZE - 14
        for i in range(len(remain) // max_part_size + 1):
            self.parts.append(
                BlockPart(remain[i * max_part_size : (i + 1) * max_part_size])
            )


@dataclass
class Header:
    pages_count: int
    first_table: tuple[int, int]
    last_table: tuple[int, int]
    cached_pages_count: int = 8192

    def __bytes__(self) -> bytes:
        buf = BytesIO()
        buf.write(b"NEOSQL")  # Magic.
        buf.write(pack_u32(self.pages_count))  # Pages count.
        buf.write(pack_addr(self.first_table))  # First table addr.
        buf.write(pack_addr(self.last_table))  # Last table addr.
        buf.write(pack_u32(self.cached_pages_count))  # Cached pages count.
        buf.write(bytes([0] * 74))  # Reserved bytes.
        return buf.getvalue()


class Page:
    free_space: int
    payload: BytesIO

    def __init__(self) -> None:
        self.free_space = PAGE_PAYLOAD_SIZE
        self.payload = BytesIO()
        self.payload.write(bytes([0] * PAGE_PAYLOAD_SIZE))
        self.payload.seek(0)

    def append_payload(self, payload: bytes) -> None:
        self.payload.write(payload)
        self.free_space -= len(payload)

    def __bytes__(self) -> bytes:
        buf = BytesIO()
        buf.write(pack_u16(self.free_space))
        buf.write(bytes([0] * 30))
        buf.write(self.payload.getvalue())
        return buf.getvalue()


def get_first_table() -> Table:
    return Table(
        name="users",
        first_record=(0, 0),
        last_record=(0, 0),
        records_count=1,
        columns=[
            Column(
                type_=DataType(DataTypeId.BIGINT, 0, False),
                name="id",
            ),
            Column(
                type_=DataType(DataTypeId.VARCHAR, 255, False),
                name="name",
            ),
            Column(
                type_=DataType(DataTypeId.VARCHAR, 256, True),
                name="surname",
            ),
        ],
    )


def get_last_table() -> Table:
    return Table(
        name="comments",
        first_record=(0, 0),
        last_record=(0, 0),
        records_count=2,
        columns=[
            Column(
                type_=DataType(DataTypeId.BIGINT, 0, False),
                name="id",
            ),
            Column(
                type_=DataType(DataTypeId.BIGINT, 0, False),
                name="author_id",
            ),
            Column(
                type_=DataType(DataTypeId.VARCHAR, 10_000, True),
                name="content",
            ),
        ],
    )


def get_first_table_first_record(table: Table) -> Record:
    return Record(
        table=table,
        values=[1, "Abobus king", None],
    )


def get_second_table_first_record(table: Table) -> Record:
    return Record(
        table=table,
        values=[1, 1, "Cool comment"],
    )


def get_second_table_second_record(table: Table) -> Record:
    return Record(
        table=table,
        values=[2, 1, "a" * 5_000],
    )


# Database layout
# | Header | Page 1 | Page 2 |

# Page 1
# | Table 1 | Table 2 | Table 1 Record 1 | Table 2 Record 1 | Table 2 Record 2 BlockPart 2 |

# Page 2
# | Table 2 Record 2 Block header | Table 2 Record 2 BlockPart 1 |


def get_t1_addr() -> tuple[int, int]:
    return (0, 0)


def get_t2_addr(t1: Table) -> tuple[int, int]:
    return (0, len(bytes(Block(bytes(t1)))))


def get_t1_r1_addr(t2_addr: tuple[int, int], t2: Table) -> tuple[int, int]:
    return (0, t2_addr[1] + len(bytes(Block(bytes(t2)))))


def get_t2_r1_addr(t1_r1_addr: tuple[int, int], t1_r1: Record) -> tuple[int, int]:
    return (0, t1_r1_addr[1] + len(bytes(Block(bytes(t1_r1)))))


def get_t2_r2_header_addr() -> tuple[int, int]:
    return (1, 0)


def get_t2_r2_part1_addr() -> tuple[int, int]:
    return (1, 15)


def get_t2_r2_part2_addr(t2_r1_addr: tuple[int, int], t2_r1: Record) -> tuple[int, int]:
    return (0, t2_r1_addr[1] + len(bytes(Block(bytes(t2_r1)))))


def main() -> None:
    t1 = get_first_table()
    t1_addr = get_t1_addr()

    t2 = get_last_table()
    t2_addr = get_t2_addr(t1)

    r1_1 = get_first_table_first_record(t1)
    t1_r1_addr = get_t1_r1_addr(t2_addr, t2)

    r2_1 = get_second_table_first_record(t2)
    t2_r1_addr = get_t2_r1_addr(t1_r1_addr, r1_1)

    r2_2 = get_second_table_second_record(t2)
    t2_r2_block_header_addr = get_t2_r2_header_addr()
    t2_r2_part1_addr = get_t2_r2_part1_addr()
    t2_r2_part2_addr = get_t2_r2_part2_addr(t2_r1_addr, r2_1)

    t1.first_record = t1_r1_addr
    t1.last_record = t1.first_record

    t2.first_record = t2_r1_addr
    t2.last_record = t2_r2_block_header_addr

    table1_block = Block(bytes(t1))
    table1_block.header.next = t2_addr

    table2_block = Block(bytes(t2))
    table2_block.header.next = NULL_ADDR

    table1_record1_block = Block(bytes(r1_1))
    table1_record1_block.header.next = NULL_ADDR

    table2_record1_block = Block(bytes(r2_1))
    table2_record1_block.header.next = t2_r2_block_header_addr

    table2_record2_block = PartedBlock(bytes(r2_2))
    table2_record2_block.header.next = NULL_ADDR
    table2_record2_block.parts[0].header.next = t2_r2_part2_addr
    table2_record2_block.parts[1].header.next = NULL_ADDR

    p1 = Page()
    p1.append_payload(bytes(table1_block))
    p1.append_payload(bytes(table2_block))
    p1.append_payload(bytes(table1_record1_block))
    p1.append_payload(bytes(table2_record1_block))
    p1.append_payload(bytes(table2_record2_block.parts[1]))

    p2 = Page()
    p2.append_payload(bytes(table2_record2_block.header))
    p2.append_payload(bytes(table2_record2_block.parts[0]))

    header = Header(
        pages_count=2,
        first_table=t1_addr,
        last_table=t2_addr,
        cached_pages_count=8192,
    )

    buf = BytesIO()
    buf.write(bytes(header))
    buf.write(bytes(p1))
    buf.write(bytes(p2))


    """ with open(VALID_DB_PATH, "wb") as fout: """
    """     fout.write(buf.getvalue()) """


if __name__ == "__main__":
    main()
