package clause

import (
	"fmt"
	"strings"
)

// SELECT col1, col2, ...
//    FROM table_name
//    WHERE [ conditions ]
//    GROUP BY col1
//    HAVING [ conditions ]

// INSERT INTO table_name(col1, col2, col3, ...) VALUES
//    (A1, A2, A3, ...),
//    (B1, B2, B3, ...),

// generator generate SQL clause (with ? wildcards) and vars
type generator func(values ...interface{}) (string, []interface{})

var generators map[Type]generator

func init() {
	generators = make(map[Type]generator)
	generators[Insert] = _insert
	generators[Values] = _values
	generators[Select] = _select
	generators[Limit] = _limit
	generators[Where] = _where
	generators[OrderBy] = _orderBy
	generators[Having] = _having
	generators[GroupBy] = _groupBy
	generators[Update] = _update
	generators[Delete] = _delete
	generators[Count] = _count
}

func genBindVars(num int) string {
	var vars []string
	for i := 0; i < num; i++ {
		vars = append(vars, "?")
	}
	return strings.Join(vars, ", ")
}

func _insert(values ...interface{}) (string, []interface{}) {
	// values[0]: table name
	// values[1:]: fields
	// INSERT INTO $tableName ($fields)
	tableName := values[0]
	fields := strings.Join(values[1].([]string), ",")
	return fmt.Sprintf("INSERT INTO %s (%v)", tableName, fields), []interface{}{}
}

func _values(values ...interface{}) (string, []interface{}) {
	// VALUES ($v1), ($v2), ...
	// $v1 is an array
	var bindStr string
	var sql strings.Builder
	var vars []interface{}
	sql.WriteString("VALUES ")
	for i, value := range values {
		v := value.([]interface{})
		if bindStr == "" {
			bindStr = genBindVars(len(v))
		}
		// (?,?,...) filled with vars
		sql.WriteString(fmt.Sprintf("(%v)", bindStr))
		if i+1 != len(values) {
			sql.WriteString(", ")
		}
		vars = append(vars, v...)
	}
	return sql.String(), vars
}

func _select(values ...interface{}) (string, []interface{}) {
	// values[0]: table name
	// values[1:]: fields
	// SELECT $fields FROM $tableName
	tableName := values[0]
	fields := strings.Join(values[1].([]string), ",")
	return fmt.Sprintf("SELECT %v FROM %s", fields, tableName), []interface{}{}
}

func _limit(values ...interface{}) (string, []interface{}) {
	// values[0]: number
	// LIMIT $num
	// ? filled with limit number
	return "LIMIT ?", values
}

func _where(values ...interface{}) (string, []interface{}) {
	// values[0]: desc with ?
	// values[1]: values to fill
	// WHERE $desc
	desc, vars := values[0], values[1:]
	return fmt.Sprintf("WHERE %s", desc), vars
}

func _orderBy(values ...interface{}) (string, []interface{}) {
	// values[0]: order
	// ORDER BY $order
	return fmt.Sprintf("ORDER BY %s", values[0]), []interface{}{}
}

func _having(values ...interface{}) (string, []interface{}) {
	// values[0]: desc with ?
	// values[1]: values to fill
	// HAVING $desc
	desc, vars := values[0], values[1:]
	return fmt.Sprintf("HAVING %s", desc), vars
}

func _groupBy(values ...interface{}) (string, []interface{}) {
	// values[0]: fields
	// GROUP BY $f1, $f2, ...
	fields := strings.Join(values[0].([]string), ",")
	return fmt.Sprintf("GROUP BY %s", fields), []interface{}{}
}

func _update(values ...interface{}) (string, []interface{}) {
	// values[0]: table
	// values[1]: map of new values
	// UPDATE $table SET $col=$val, $col=$val...
	tableName := values[0]
	m := values[1].(map[string]interface{})
	var keys []string
	var vars []interface{}
	for k, v := range m {
		keys = append(keys, k+" = ?")
		vars = append(vars, v)
	}
	return fmt.Sprintf("UPDATE %s SET %s", tableName, strings.Join(keys, ", ")), vars
}

func _delete(values ...interface{}) (string, []interface{}) {
	// values[0]: table
	// DELETE FROM $table
	return fmt.Sprintf("DELETE FROM %s", values[0]), []interface{}{}
}

func _count(values ...interface{}) (string, []interface{}) {
	// SELECT count(*)
	return _select(values[0], []string{"count(*)"})
}
