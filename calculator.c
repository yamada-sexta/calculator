#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

// Global widgets and state variables
GtkWidget *entry;
double first_num = 0.0;
char current_op = ' ';
gboolean new_entry = TRUE;

// --- Calculation Logic ---

// Performs the calculation when '=' is pressed.
void perform_calculation() {
    const char *entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
    double second_num = atof(entry_text);
    double result = 0.0;
    char result_str[256];

    switch (current_op) {
        case '+':
            result = first_num + second_num;
            break;
        case '-':
            result = first_num - second_num;
            break;
        case '*':
            result = first_num * second_num;
            break;
        case '/':
            if (second_num != 0) {
                result = first_num / second_num;
            } else {
                // Handle division by zero
                gtk_entry_set_text(GTK_ENTRY(entry), "Error: Div by 0");
                // Reset state
                first_num = 0.0;
                current_op = ' ';
                new_entry = TRUE;
                return;
            }
            break;
        default:
            // If no operator was pressed, do nothing.
            return;
    }

    // Format the result and display it.
    g_snprintf(result_str, sizeof(result_str), "%g", result);
    gtk_entry_set_text(GTK_ENTRY(entry), result_str);

    // Reset state for the next calculation
    first_num = 0.0;
    current_op = ' ';
    new_entry = TRUE;
}


// --- Callback Functions for Buttons ---

// Called when a number button (0-9) or the decimal point is clicked.
void on_number_button_clicked(GtkButton *button, gpointer user_data) {
    const char *button_label = gtk_button_get_label(button);

    // If this is the start of a new number (after an operator or '='), clear the entry first.
    if (new_entry) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
        new_entry = FALSE;
    }

    // Append the new digit/decimal to the entry.
    const char *current_text = gtk_entry_get_text(GTK_ENTRY(entry));
    char *new_text = g_strconcat(current_text, button_label, NULL);
    gtk_entry_set_text(GTK_ENTRY(entry), new_text);
    g_free(new_text);
}

// Called when an operator button (+, -, *, /) is clicked.
void on_operator_button_clicked(GtkButton *button, gpointer user_data) {
    const char *op_str = gtk_button_get_label(button);
    const char *entry_text = gtk_entry_get_text(GTK_ENTRY(entry));

    // If an operator is pressed again before entering a second number,
    // just update the operator. Otherwise, perform the pending calculation.
    if (current_op != ' ' && !new_entry) {
        perform_calculation();
    }
    
    // Store the first number and the operator.
    first_num = atof(entry_text);
    current_op = op_str[0];
    new_entry = TRUE; // The next number typed will be a new entry.
}

// Called when the equals button (=) is clicked.
void on_equals_button_clicked(GtkButton *button, gpointer user_data) {
    if (current_op != ' ') {
        perform_calculation();
    }
}

// Called when the clear button (C) is clicked.
void on_clear_button_clicked(GtkButton *button, gpointer user_data) {
    gtk_entry_set_text(GTK_ENTRY(entry), "0");
    first_num = 0.0;
    current_op = ' ';
    new_entry = TRUE;
}gcc calculator.c -o calculator $(pkg-config --cflags --libs gtk+-3.0)
// Creates and sets up all the widgets in the window.
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button;

    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 300);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Create a grid to arrange the widgets
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    // Create the display entry
    entry = gtk_entry_new();
    gtk_entry_set_alignment(GTK_ENTRY(entry), 1.0); // Right-align text
    gtk_widget_set_sensitive(entry, FALSE); // Make it read-only
    gtk_entry_set_text(GTK_ENTRY(entry), "0");

    // Attach the entry to the grid, making it span 4 columns
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 4, 1);

    // Define the button labels in a 2D array for easy layout
    const char *buttons[5][4] = {
        {"C", "()", "%", "/"},
        {"7", "8", "9", "*"},
        {"4", "5", "6", "-"},
        {"1", "2", "3", "+"},
        {"0", ".", "=", NULL}
    };

    // Create and attach buttons
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            if (buttons[i][j] == NULL) continue;

            button = gtk_button_new_with_label(buttons[i][j]);
            gtk_widget_set_hexpand(button, TRUE);
            gtk_widget_set_vexpand(button, TRUE);
            
            const char *label = buttons[i][j];

            // Connect the appropriate signal based on the button label
            if (g_str_equal(label, "C")) {
                g_signal_connect(button, "clicked", G_CALLBACK(on_clear_button_clicked), NULL);
            } else if (g_str_equal(label, "=")) {
                g_signal_connect(button, "clicked", G_CALLBACK(on_equals_button_clicked), NULL);
            } else if (strchr("+-*/", label[0]) && strlen(label) == 1) {
                g_signal_connect(button, "clicked", G_CALLBACK(on_operator_button_clicked), NULL);
            } else if (strchr("0123456789.", label[0])) {
                g_signal_connect(button, "clicked", G_CALLBACK(on_number_button_clicked), NULL);
            }
            // Note: The "()" and "%" buttons are not implemented in this simple example.

            // Handle special cases for button spanning
            if (g_str_equal(label, "0")) {
                gtk_grid_attach(GTK_GRID(grid), button, j, i + 1, 2, 1);
                j++; // Skip the next grid cell
            } else {
                gtk_grid_attach(GTK_GRID(grid), button, j, i + 1, 1, 1);
            }
        }
    }

    // Show all widgets in the window
    gtk_widget_show_all(window);
}


int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Create a new GTK application
    app = gtk_application_new("org.gtk.example.calculator", G_APPLICATION_DEFAULT_FLAGS);
    
    // Connect the 'activate' signal to the activate function
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    // Run the application
    status = g_application_run(G_APPLICATION(app), argc, argv);
    
    // Free the application object
    g_object_unref(app);

    return status;
}
